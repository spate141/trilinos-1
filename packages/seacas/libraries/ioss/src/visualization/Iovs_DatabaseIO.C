/*--------------------------------------------------------------------*/
/*    Copyright 2000-2010 Sandia Corporation.                         */
/*    Under the terms of Contract DE-AC04-94AL85000, there is a       */
/*    non-exclusive license for use of this work by or on behalf      */
/*    of the U.S. Government.  Export of this program may require     */
/*    a license from the United States Government.                    */
/*--------------------------------------------------------------------*/

#include <Ioss_CodeTypes.h>
#include <visualization/Iovs_DatabaseIO.h>
#include <tokenize.h>
#include <ParaViewCatalystSierraAdaptor.h>

#include <cstring>
#include <cctype>
#include <cstdlib>
#include <iterator>

#include <Ioss_SubSystem.h>
#include <Ioss_Utils.h>
#include <Ioss_ParallelUtils.h>
#include <Ioss_SerializeIO.h>
#include <Ioss_ElementTopology.h>
#include <Ioss_FileInfo.h>
#include <Ioss_SurfaceSplit.h>

#include <assert.h>

//#include <exodusII.h>
  enum ex_entity_type {
    EX_NODAL       = 14,          /**< nodal "block" for variables*/
    EX_NODE_BLOCK  = 14,          /**< alias for EX_NODAL         */
    EX_NODE_SET    =  2,          /**< node set property code     */
    EX_EDGE_BLOCK  =  6,          /**< edge block property code   */
    EX_EDGE_SET    =  7,          /**< edge set property code     */
    EX_FACE_BLOCK  =  8,          /**< face block property code   */
    EX_FACE_SET    =  9,          /**< face set property code     */
    EX_ELEM_BLOCK  =  1,          /**< element block property code*/
    EX_ELEM_SET    = 10,          /**< face set property code     */

    EX_SIDE_SET    =  3,          /**< side set property code     */

    EX_ELEM_MAP    =  4,          /**< element map property code  */
    EX_NODE_MAP    =  5,          /**< node map property code     */
    EX_EDGE_MAP    = 11,          /**< edge map property code     */
    EX_FACE_MAP    = 12,          /**< face map property code     */

    EX_GLOBAL      = 13,          /**< global "block" for variables*/
    EX_COORDINATE  = 15,          /**< kluge so some internal wrapper functions work */
    EX_INVALID     = -1};
  typedef enum ex_entity_type ex_entity_type;

namespace Iovs {
  int field_warning(const Ioss::GroupingEntity *ge,
                    const Ioss::Field &field, const std::string& inout);

  DatabaseIO::DatabaseIO(Ioss::Region *region, const std::string& filename,
                         Ioss::DatabaseUsage db_usage, MPI_Comm communicator,
			 const Ioss::PropertyManager &props) :
    Ioss::DatabaseIO (region, filename, db_usage, communicator, props)

  {
    dbState = Ioss::STATE_UNKNOWN;
    this->pvcsa = 0;
    this->globalNodeAndElementIDsCreated = false;

    if(props.exists("VISUALIZATION_SCRIPT"))
      {
      this->paraview_script_filename = props.get("VISUALIZATION_SCRIPT").get_string();
      }
    else
      {
      std::ostringstream errmsg;
      errmsg << "Property VISUALIZATION_SCRIPT not given in results output \n"
             << "block, unable to initialize ParaView Catalyst";
      IOSS_ERROR(errmsg);
      }
  }

  DatabaseIO::~DatabaseIO() 
  {
    try {
      if(this->pvcsa)
        delete this->pvcsa;
    } catch (...) {
    }
  }

  void DatabaseIO::release_memory()
  {
    nodeMap.release_memory();
    elemMap.release_memory();
  }

  bool DatabaseIO::begin(Ioss::State state)
  {
    dbState = state;

    Ioss::Region *region = this->get_region();
    if(region->model_defined() && !this->pvcsa)
      {
      this->pvcsa = ParaViewCatalystSierraAdaptorBaseFactory::create("ParaViewCatalystSierraAdaptor")();
      if(this->pvcsa)
        this->pvcsa->InitializeParaViewCatalyst(this->paraview_script_filename.c_str());
      std::vector<int> element_block_id_list;
      Ioss::ElementBlockContainer const & ebc = region->get_element_blocks();
      for(int i = 0;i<ebc.size();i++)
        {
        if (ebc[i]->property_exists("id"))
          element_block_id_list.push_back(ebc[i]->get_property("id").get_int());
        }
      if(this->pvcsa)
        this->pvcsa->InitializeElementBlocks(element_block_id_list);
      }
    return true;
  }

  bool DatabaseIO::end(Ioss::State state)
  {
    // Transitioning out of state 'state'
    assert(state == dbState);
    switch (state) {
    case Ioss::STATE_DEFINE_MODEL:
      write_meta_data();
      break;
    case Ioss::STATE_DEFINE_TRANSIENT:
      // TODO, is there metadata we can prep through ITAPS?
      // write_results_metadata();
      break;
    default: // ignore everything else...
      break;
    }

    {
      Ioss::SerializeIO serializeIO__(this);
      dbState = Ioss::STATE_UNKNOWN;
    }

    return true;
  }

  // Default versions do nothing at this time...
  // Will be used for global variables...
  bool DatabaseIO::begin_state(Ioss::Region* region, int state, double time)
  {
    Ioss::SerializeIO   serializeIO__(this);

    if(!this->globalNodeAndElementIDsCreated)
      {
      this->create_global_node_and_element_ids();
      }

    if(this->pvcsa)
      this->pvcsa->SetTimeData(time, state - 1);

    // Zero global variable array...
    // std::fill(globalValues.begin(), globalValues.end(), 0.0);
    return true;
  }

  bool DatabaseIO::end_state(Ioss::Region*, int state, double time)
  {
    Ioss::SerializeIO   serializeIO__(this);

    if(this->pvcsa)
      {
      this->pvcsa->PerformCoProcessing();
      this->pvcsa->ReleaseMemory();
      }

    return true;
  }

  void DatabaseIO::read_meta_data ()
  {
    // TODO fillin
  }

  void DatabaseIO::create_global_node_and_element_ids()
  {
  Ioss::ElementBlockContainer element_blocks = this->get_region()->get_element_blocks();
  Ioss::ElementBlockContainer::const_iterator I;
  std::vector<std::string> component_names;
  std::vector<std::string> element_block_name;
  element_block_name.push_back("ElementBlockIds");
  component_names.push_back("GlobalElementId");
  for (I=element_blocks.begin(); I != element_blocks.end(); ++I)
    {
    if ((*I)->property_exists("id"))
      {
      int64_t eb_offset = (*I)->get_offset();
      int bid = (*I)->get_property("id").get_int();
      if(this->pvcsa)
        this->pvcsa->CreateGlobalVariable(element_block_name,
                                          bid,
                                          &bid);
      if(this->pvcsa)
        this->pvcsa->CreateElementVariable(component_names,
                                           bid,
                                           &this->elemMap.map[eb_offset + 1]);
      }
    }

  component_names.clear();
  component_names.push_back("GlobalNodeId");
  if(this->pvcsa)
    this->pvcsa->CreateNodalVariable(component_names,
                                     &this->nodeMap.map[1]);

  this->globalNodeAndElementIDsCreated = true;
  }

  //------------------------------------------------------------------------
  int64_t DatabaseIO::put_field_internal(const Ioss::Region* /* region */,
                                         const Ioss::Field& field,
                                         void *data, size_t data_size) const
  {
    // For now, assume that all TRANSIENT fields on a region
    // are REDUCTION fields (1 value).  We need to gather these
    // and output them all at one time.  The storage location is a
    // 'globalVariables' array
      Ioss::SerializeIO serializeIO__(this);

      size_t num_to_get = field.verify(data_size);
      Ioss::Field::RoleType role = field.get_role();
      const Ioss::VariableType *var_type = field.transformed_storage();
      if ((role == Ioss::Field::TRANSIENT || role == Ioss::Field::REDUCTION) &&
          num_to_get == 1) {
        const char *complex_suffix[] = {".re", ".im"};
        Ioss::Field::BasicType ioss_type = field.get_type();
        double  *rvar = static_cast<double*>(data);
        int     *ivar = static_cast<int*>(data);
        int64_t *ivar64 = static_cast<int64_t*>(data);

        int comp_count = var_type->component_count();
        int var_index=0;

        int re_im = 1;
        if (field.get_type() == Ioss::Field::COMPLEX)
          re_im = 2;
        for (int complex_comp = 0; complex_comp < re_im; complex_comp++) {
          std::string field_name = field.get_name();
          if (re_im == 2) {
            field_name += complex_suffix[complex_comp];
          }

          std::vector<std::string> component_names;
          std::vector<double> globalValues;
          char field_suffix_separator = get_field_separator();
          for (int i=0; i < comp_count; i++) {
            std::string var_name = var_type->label_name(field_name, i+1, field_suffix_separator);
            component_names.push_back(var_name);

            // Transfer from 'variables' array.
            if (ioss_type == Ioss::Field::REAL || ioss_type == Ioss::Field::COMPLEX)
              globalValues.push_back(rvar[i]);
            else if (ioss_type == Ioss::Field::INTEGER)
              globalValues.push_back(ivar[i]);
            else if (ioss_type == Ioss::Field::INT64)
              globalValues.push_back(ivar64[i]);
          }
          if(this->pvcsa)
            this->pvcsa->CreateGlobalVariable(component_names,
                                              TOPTR(globalValues));
        }
      }
      else if (num_to_get != 1) {
        // There should have been a warning/error message printed to the
        // log file earlier for this, so we won't print anything else
        // here since it would be printed for each and every timestep....
        ;
      } else {
        std::ostringstream errmsg;
        errmsg << "The variable named '" << field.get_name()
               << "' is of the wrong type. A region variable must be of type"
               << " TRANSIENT or REDUCTION.\n"
               << "This is probably an internal error; please notify gdsjaar@sandia.gov";
        IOSS_ERROR(errmsg);
      }
      return num_to_get;
  }

  int64_t DatabaseIO::put_field_internal(const Ioss::NodeBlock* nb,
                                         const Ioss::Field& field,
                                         void *data,
                                         size_t data_size) const
  {
      Ioss::SerializeIO serializeIO__(this);

      size_t num_to_get = field.verify(data_size);
      if (num_to_get > 0) {
        Ioss::Field::RoleType role = field.get_role();

        if (role == Ioss::Field::MESH) {
          if (field.get_name() == "mesh_model_coordinates") {
            if(this->pvcsa)
              this->pvcsa->InitializeGlobalPoints(num_to_get,
                                                  static_cast<double*>(data));
          } else if (field.get_name() == "ids") {
            // The ids coming in are the global ids; their position is the
            // local id -1 (That is, data[0] contains the global id of local
            // node 1)

            // Another 'const-cast' since we are modifying the database just
            // for efficiency; which the client does not see...
            DatabaseIO *new_this = const_cast<DatabaseIO*>(this);
            /*64 bit should be okay*/
            new_this->handle_node_ids(data, num_to_get);
          } else if (field.get_name() == "connectivity") {
            // Do nothing, just handles an idiosyncracy of the GroupingEntity
          } else {
            return field_warning(nb, field, "mesh output");
          }
        } else if (role == Ioss::Field::TRANSIENT) {
          const char *complex_suffix[] = {".re", ".im"};
          Ioss::Field::BasicType ioss_type = field.get_type();
          const Ioss::VariableType *var_type = field.transformed_storage();
          std::vector<double> temp(num_to_get);
          int comp_count = var_type->component_count();
          int re_im = 1;
          if (ioss_type == Ioss::Field::COMPLEX)
            re_im = 2;
          for (int complex_comp = 0; complex_comp < re_im; complex_comp++) {
            std::string field_name = field.get_name();
            if (re_im == 2) {
              field_name += complex_suffix[complex_comp];
            }

            std::vector<double> interleaved_data(num_to_get*comp_count);
            std::vector<std::string> component_names;
            char field_suffix_separator = get_field_separator();
            for (int i=0; i < comp_count; i++)
              {
              std::string var_name = var_type->label_name(field_name, i+1, field_suffix_separator);
              component_names.push_back(var_name);

              size_t begin_offset = (re_im*i)+complex_comp;
              size_t stride = re_im*comp_count;

              if (ioss_type == Ioss::Field::REAL || ioss_type == Ioss::Field::COMPLEX)
                this->nodeMap.map_field_to_db_scalar_order(static_cast<double*>(data),
                    temp, begin_offset, num_to_get, stride, 0);
              else if (ioss_type == Ioss::Field::INTEGER)
                this->nodeMap.map_field_to_db_scalar_order(static_cast<int*>(data),
                    temp, begin_offset, num_to_get, stride, 0);
              else if (ioss_type == Ioss::Field::INT64)
                this->nodeMap.map_field_to_db_scalar_order(static_cast<int64_t*>(data),
                    temp, begin_offset, num_to_get, stride, 0);

              for(int j=0; j < num_to_get; j++)
                interleaved_data[j*comp_count + i] = temp[j];
              }

            if(this->pvcsa)
              this->pvcsa->CreateNodalVariable(component_names,
                                               TOPTR(interleaved_data));
          }
        } else if (role == Ioss::Field::REDUCTION) {
          // TODO imesh version
          // write_global_field(EX_NODAL, field, nb, data);
        }
      }
      return num_to_get;
  }

  int64_t DatabaseIO::put_field_internal(const Ioss::ElementBlock* eb,
                                         const Ioss::Field& field,
                                         void *data, size_t data_size) const
  {
      Ioss::SerializeIO serializeIO__(this);

      if (!eb->property_exists("id"))
        return 0;

      size_t num_to_get = field.verify(data_size);
      if (num_to_get > 0) {

        // Get the element block id and element count
        
        // TODO replace with legit block id
        // int64_t id = get_id(eb, EX_ELEM_BLOCK, &ids_);
        int64_t element_count = eb->get_property("entity_count").get_int();
        Ioss::Field::RoleType role = field.get_role();

        if (role == Ioss::Field::MESH) {
          // Handle the MESH fields required for an ExodusII file model.
          // (The 'genesis' portion)
          if (field.get_name() == "connectivity") {
            if (element_count > 0) {
              // Map element connectivity from global node id to local node id.
              // Do it in 'data' ...
              int element_nodes = eb->get_property("topology_node_count").get_int();
              assert(field.transformed_storage()->component_count() == element_nodes);
              nodeMap.reverse_map_data(data, field, num_to_get*element_nodes);
                  int bid = 0;
                  if (eb->property_exists("id"))
                     bid = eb->get_property("id").get_int();
                  if(this->pvcsa)
                    this->pvcsa->CreateElementBlock(eb->name().c_str(),
                                                    bid,
                                                    element_nodes,
                                                    num_to_get,
                                                    static_cast<int*>(data));
            }
          } else if (field.get_name() == "ids") {
            // Another 'const-cast' since we are modifying the database just
            // for efficiency; which the client does not see...
            DatabaseIO *new_this = const_cast<DatabaseIO*>(this);
            new_this->handle_element_ids(eb, data, num_to_get);

          } else if (field.get_name() == "skin") {
	    // Not applicable to viz output.
          } else {
            IOSS_WARNING << " ElementBlock " << eb->name()
                         << ". Unknown field " << field.get_name();
            num_to_get = 0;
          }
        }
        else if (role == Ioss::Field::ATTRIBUTE) {
                /* TODO */
        } else if (role == Ioss::Field::TRANSIENT) {
          const char *complex_suffix[] = {".re", ".im"};
          const Ioss::VariableType *var_type = field.transformed_storage();
          Ioss::Field::BasicType ioss_type = field.get_type();
          std::vector<double> temp(num_to_get);
          ssize_t eb_offset = eb->get_offset();
          int comp_count = var_type->component_count();
          int bid = 0;
          if (eb->property_exists("id"))
             bid = eb->get_property("id").get_int();

          int re_im = 1;
          if (ioss_type == Ioss::Field::COMPLEX)
            re_im = 2;
          for (int complex_comp = 0; complex_comp < re_im; complex_comp++) {
            std::string field_name = field.get_name();
            if (re_im == 2) {
              field_name += complex_suffix[complex_comp];
            }

            std::vector<double> interleaved_data(num_to_get*comp_count);
            std::vector<std::string> component_names;
            char field_suffix_separator = get_field_separator();
            for (int i=0; i < comp_count; i++) {
              std::string var_name = var_type->label_name(field_name, i+1, field_suffix_separator);
              component_names.push_back(var_name);

              ssize_t begin_offset = (re_im*i)+complex_comp;
              ssize_t stride = re_im*comp_count;

              if (ioss_type == Ioss::Field::REAL || ioss_type == Ioss::Field::COMPLEX)
                this->elemMap.map_field_to_db_scalar_order(static_cast<double*>(data),
                    temp, begin_offset, num_to_get, stride, eb_offset);
              else if (ioss_type == Ioss::Field::INTEGER)
                this->elemMap.map_field_to_db_scalar_order(static_cast<int*>(data),
                    temp, begin_offset, num_to_get, stride, eb_offset);
              else if (ioss_type == Ioss::Field::INT64)
                this->elemMap.map_field_to_db_scalar_order(static_cast<int64_t*>(data),
                    temp, begin_offset, num_to_get, stride, eb_offset);
              for(int j=0; j < num_to_get; j++)
                 interleaved_data[j*comp_count + i] = temp[j];
            }
            if(this->pvcsa)
               this->pvcsa->CreateElementVariable(component_names,
                                                  bid,
                                                  TOPTR(interleaved_data));
            }
        } else if (role == Ioss::Field::REDUCTION) {
          // TODO replace with ITAPS
          // write_global_field(EX_ELEM_BLOCK, field, eb, data);
        }
      }
      return num_to_get;
  }

  void DatabaseIO::write_meta_data ()
  {
    Ioss::Region *region = get_region();

    // Node Blocks --
    {
      //std::cout << "DatabaseIO::write_meta_data node blocks\n";
      Ioss::NodeBlockContainer node_blocks = region->get_node_blocks();
      assert(node_blocks.size() == 1);
      spatialDimension = node_blocks[0]->
                           get_property("component_degree").get_int();
      nodeCount =        node_blocks[0]->
                           get_property("entity_count").get_int();
      //std::cout << "DatabaseIO::write_meta_data nodeCount:" << nodeCount << "\n";
    }
    
    // Element Blocks --
    {
      //std::cout << "DatabaseIO::write_meta_data element blocks test1\n";
      Ioss::ElementBlockContainer element_blocks =
                                    region->get_element_blocks();
      // assert(check_block_order(element_blocks));
      Ioss::ElementBlockContainer::const_iterator I;
      elementBlockCount = 0;
      elementCount = 0;
      //std::cout << "DatabaseIO::write_meta_data element num blocks:" << element_blocks.size() << "\n";
      for (I=element_blocks.begin(); I != element_blocks.end(); ++I) {
        elementBlockCount ++;
        elementCount += (*I)->get_property("entity_count").get_int();
        //std::cout << "DatabaseIO::write_meta_data element num in block " << elementBlockCount << ": " << (*I)->get_property("entity_count").get_int() << "\n";
      }
      //std::cout << "DatabaseIO::write_meta_data elementCount:" << elementCount << "\n";
    }
    //std::cout << "DatabaseIO::write_meta_data returning\n";
  }

  int64_t DatabaseIO::handle_node_ids(void* ids, int64_t num_to_get)
  {
    //std::cout << "DatabaseIO::handle_node_ids executing\n";
    /*!
     * There are two modes we need to support in this routine:
     * 1. Initial definition of node map (local->global) and
     * reverseNodeMap (global->local).
     * 2. Redefinition of node map via 'reordering' of the original
     * map when the nodes on this processor are the same, but their
     * order is changed (or count because of ghosting)
     *
     * So, there will be two maps the 'nodeMap' map is a 'direct lookup'
     * map which maps current local position to global id and the
     * 'reverseNodeMap' is an associative lookup which maps the
     * global id to 'original local'.  There is also a
     * 'reorderNodeMap' which is direct lookup and maps current local
     * position to original local.

     * The ids coming in are the global ids; their position is the
     * "local id-1" (That is, data[0] contains the global id of local
     * node 1 in this node block).
     *
     * int local_position = reverseNodeMap[NodeMap[i+1]]
     * (the nodeMap and reverseNodeMap are 1-based)
     *
     * To determine which map to update on a call to this function, we
     * use the following hueristics:
     * -- If the database state is 'STATE_MODEL:', then update the
     *    'reverseNodeMap' and 'nodeMap'
     *
     * -- If the database state is not STATE_MODEL, then leave the
     *    'reverseNodeMap' and 'nodeMap' alone since they correspond to the
     *    information already written to the database. [May want to add a
     *    STATE_REDEFINE_MODEL]
     *
     * -- In both cases, update the reorderNodeMap
     *
     * NOTE: The mapping is done on TRANSIENT fields only; MODEL fields
     *       should be in the orginal order...
     */
    assert(num_to_get == nodeCount);

    if (dbState == Ioss::STATE_MODEL) {
      if (nodeMap.map.empty()) {
        //std::cout << "DatabaseIO::handle_node_ids nodeMap was empty, resizing and tagging serial\n";
        nodeMap.map.resize(nodeCount+1);
	nodeMap.map[0] = -1;
      }

      if (nodeMap.map[0] == -1) {
        //std::cout << "DatabaseIO::handle_node_ids nodeMap tagged serial, doing mapping\n";
	if (int_byte_size_api() == 4) {
	  nodeMap.set_map(static_cast<int*>(ids), num_to_get, 0);
	} else {
	  nodeMap.set_map(static_cast<int64_t*>(ids), num_to_get, 0);
	}	    
      }

	nodeMap.build_reverse_map(myProcessor);

	// Only a single nodeblock and all set
	if (num_to_get == nodeCount) {
	  assert(nodeMap.map[0] == -1 || nodeMap.reverse.size() == (size_t)nodeCount);
	}
	assert(get_region()->get_property("node_block_count").get_int() == 1);
      }

      nodeMap.build_reorder_map(0, num_to_get);
      return num_to_get;
    }

      size_t handle_block_ids(const Ioss::EntityBlock *eb,
			      ex_entity_type map_type,
			      Ioss::State db_state,
			      Ioss::Map &entity_map,
			      void* ids, size_t int_byte_size, size_t num_to_get, /*int file_pointer,*/ int my_processor)
      {
        //std::cout << "DatabaseIO::handle_block_ids executing\n";
	/*!
	 * NOTE: "element" is generic for "element", "face", or "edge"
	 *
	 * There are two modes we need to support in this routine:
	 * 1. Initial definition of element map (local->global) and
	 * reverseElementMap (global->local).
	 * 2. Redefinition of element map via 'reordering' of the original
	 * map when the elements on this processor are the same, but their
	 * order is changed.
	 *
	 * So, there will be two maps the 'elementMap' map is a 'direct lookup'
	 * map which maps current local position to global id and the
	 * 'reverseElementMap' is an associative lookup which maps the
	 * global id to 'original local'.  There is also a
	 * 'reorderElementMap' which is direct lookup and maps current local
	 * position to original local.

	 * The ids coming in are the global ids; their position is the
	 * local id -1 (That is, data[0] contains the global id of local
	 * element 1 in this element block).  The 'model-local' id is
	 * given by eb_offset + 1 + position:
	 *
	 * int local_position = reverseElementMap[ElementMap[i+1]]
	 * (the elementMap and reverseElementMap are 1-based)
	 *
	 * But, this assumes 1..numel elements are being output at the same
	 * time; we are actually outputting a blocks worth of elements at a
	 * time, so we need to consider the block offsets.
	 * So... local-in-block position 'i' is index 'eb_offset+i' in
	 * 'elementMap' and the 'local_position' within the element
	 * blocks data arrays is 'local_position-eb_offset'.  With this, the
	 * position within the data array of this element block is:
	 *
	 * int eb_position =
	 * reverseElementMap[elementMap[eb_offset+i+1]]-eb_offset-1
	 *
	 * To determine which map to update on a call to this function, we
	 * use the following hueristics:
	 * -- If the database state is 'Ioss::STATE_MODEL:', then update the
	 *    'reverseElementMap'.
	 * -- If the database state is not Ioss::STATE_MODEL, then leave
	 *    the 'reverseElementMap' alone since it corresponds to the
	 *    information already written to the database. [May want to add
	 *    a Ioss::STATE_REDEFINE_MODEL]
	 * -- Always update elementMap to match the passed in 'ids'
	 *    array.
	 *
	 * NOTE: the maps are built an element block at a time...
	 * NOTE: The mapping is done on TRANSIENT fields only; MODEL fields
	 *       should be in the orginal order...
	 */

	// Overwrite this portion of the 'elementMap', but keep other
	// parts as they were.  We are adding elements starting at position
	// 'eb_offset+offset' and ending at
	// 'eb_offset+offset+num_to_get'. If the entire block is being
	// processed, this reduces to the range 'eb_offset..eb_offset+my_element_count'

	int64_t eb_offset = eb->get_offset();

	if (int_byte_size == 4) {
	  entity_map.set_map(static_cast<int*>(ids), num_to_get, eb_offset);
	} else {
	  entity_map.set_map(static_cast<int64_t*>(ids), num_to_get, eb_offset);
	}

	// Now, if the state is Ioss::STATE_MODEL, update the reverseEntityMap
	if (db_state == Ioss::STATE_MODEL) {
	  entity_map.build_reverse_map(num_to_get, eb_offset, my_processor);
        }

	// Build the reorderEntityMap which does a direct mapping from
	// the current topologies local order to the local order
	// stored in the database...  This is 0-based and used for
	// remapping output and input TRANSIENT fields.
	entity_map.build_reorder_map(eb_offset, num_to_get);
        //std::cout << "DatabaseIO::handle_block_ids returning\n";
	return num_to_get;
      }

  int64_t DatabaseIO::handle_element_ids(const Ioss::ElementBlock *eb, void* ids, size_t num_to_get)
  {
      //std::cout << "DatabaseIO::handle_element_ids executing num_to_get: " << num_to_get << "\n";
      if (elemMap.map.empty()) {
        //std::cout << "DatabaseIO::handle_element_ids elementMap was empty; allocating and marking as sequential\nelmenetCount: " << elementCount << "\n";
        elemMap.map.resize(elementCount+1);
        elemMap.map[0] = -1;
      }
      //std::cout << "DatabaseIO::handle_element_ids elementMap size: " << elementMap.size() << "\n";
      return handle_block_ids(eb, EX_ELEM_MAP, dbState, elemMap,
                              ids, int_byte_size_api(), num_to_get, /*get_file_pointer(),*/ myProcessor);
  }


  const Ioss::Map& DatabaseIO::get_node_map() const
  {
    //std::cout << "in new nathan Iovs DatabaseIO::get_node_reorder_map\n";
    // Allocate space for node number map and read it in...
    // Can be called multiple times, allocate 1 time only
    if (nodeMap.map.empty()) {
      //std::cout << "DatabaseIO::get_node_map  nodeMap was empty, resizing and tagging sequential\n";
      nodeMap.map.resize(nodeCount+1);

      // Output database; nodeMap not set yet... Build a default map.
      for (int64_t i=1; i < nodeCount+1; i++) {
	nodeMap.map[i] = i;
      }
      // Sequential map
      nodeMap.map[0] = -1;
    }
    return nodeMap;
  }

  // Not used...
  const Ioss::Map& DatabaseIO::get_element_map() const
  {
    //std::cout << "in new nathan Iovs DatabaseIO::get_element_map\n";
    // Allocate space for elemente number map and read it in...
    // Can be called multiple times, allocate 1 time only
    if (elemMap.map.empty()) {
      elemMap.map.resize(elementCount+1);

	// Output database; elementMap not set yet... Build a default map.
	for (int64_t i=1; i < elementCount+1; i++) {
	  elemMap.map[i] = i;
	}
	// Sequential map
	elemMap.map[0] = -1;
    }
    return elemMap;
  }

  int field_warning(const Ioss::GroupingEntity *ge,
                    const Ioss::Field &field, const std::string& inout)
  {
    IOSS_WARNING << ge->type() << " '" << ge->name()
                 << "'. Unknown " << inout << " field '"
                 << field.get_name() << "'";
    return -4;
  }

};
