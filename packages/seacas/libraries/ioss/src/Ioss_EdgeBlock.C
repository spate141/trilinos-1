// Copyright(C) 1999-2010
// Sandia Corporation. Under the terms of Contract
// DE-AC04-94AL85000 with Sandia Corporation, the U.S. Government retains
// certain rights in this software.
//         
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
// 
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
// 
//     * Redistributions in binary form must reproduce the above
//       copyright notice, this list of conditions and the following
//       disclaimer in the documentation and/or other materials provided
//       with the distribution.
//     * Neither the name of Sandia Corporation nor the names of its
//       contributors may be used to endorse or promote products derived
//       from this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <Ioss_EdgeBlock.h>
#include <Ioss_DatabaseIO.h>
#include <Ioss_Property.h>
#include <Ioss_Field.h>
#include <Ioss_VariableType.h>
#include <Ioss_ElementTopology.h>

#include <string>

Ioss::EdgeBlock::EdgeBlock(const Ioss::DatabaseIO *io_database,
			   const std::string& my_name,
			   const std::string& edge_type,
			   size_t number_edges)
  : Ioss::EntityBlock(io_database, my_name, edge_type, number_edges)
{
  if (topology()->master_element_name() != edge_type &&
      topology()->name() != edge_type) {
    // Maintain original edge type on output database if possible.
    properties.add(Ioss::Property("original_edge_type", edge_type));
  }

}

Ioss::EdgeBlock::~EdgeBlock() {}

Ioss::Property Ioss::EdgeBlock::get_implicit_property(const std::string& my_name) const
{
  return Ioss::EntityBlock::get_implicit_property(my_name);
}

int Ioss::EdgeBlock::internal_get_field_data(const Ioss::Field& field,
				      void *data, size_t data_size) const
{
  return get_database()->get_field(this, field, data, data_size);
}

int Ioss::EdgeBlock::internal_put_field_data(const Ioss::Field& field,
				      void *data, size_t data_size) const
{
  return get_database()->put_field(this, field, data, data_size);
}