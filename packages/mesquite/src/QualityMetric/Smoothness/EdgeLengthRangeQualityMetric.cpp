/*! \file EdgeLengthRangeQualityMetric.cpp
  \author Michael Brewer
  \date 2002-05-14
  Evaluates the lengths of the edges attached to the given vertex.
  By default, the averaging method is set to SUM.
*/


#include "EdgeLengthRangeQualityMetric.hpp"
#include <math.h>
#include <list>
#include "Vector3D.hpp"
#include "QualityMetric.hpp"
#include "MsqVertex.hpp"
#include "PatchData.hpp"
#include "MsqMessage.hpp"
using namespace Mesquite;


#undef __FUNC__
#define __FUNC__ "EdgeLengthRangeQualityMetric::evaluate_node"
/*!For the given vertex, vert, with connected edges of lengths l_j for
  j=1...k, the metric value is the average (where the default average
  type is SUM) of
        u_j = ( | l_j - lowVal | - (l_j - lowVal) )^2 +
              ( | highVal - l_j | - (highVal - l_j) )^2.
*/
bool EdgeLengthRangeQualityMetric::evaluate_vertex(PatchData &pd, MsqVertex* vert,
                                             double &fval, MsqError &err)
{
  fval=0.0;
  size_t this_vert = pd.get_vertex_index(vert);
  size_t other_vert;
  std::vector<size_t> adj_verts;
  Vector3D edg;
  pd.get_adjacent_vertex_indices(this_vert,adj_verts,err);
  int num_sample_points=adj_verts.size();
  double *metric_values=new double[num_sample_points];
  MsqVertex* verts = pd.get_vertex_array(err);
  int point_counter=0;
    //store the length of the edge, and the first and second component of
    //metric values, respectively.
  double temp_length=0.0;
  double temp_first=0.0;
  double temp_second=0.0;
    //PRINT_INFO("INSIDE ELR, vertex = %f,%f,%f\n",verts[this_vert][0],verts[this_vert][1],verts[this_vert][2]);
    //loop while there are still more adjacent vertices.
  while(!adj_verts.empty()){
    other_vert=adj_verts.back();
    adj_verts.pop_back();
    edg[0]=verts[this_vert][0]-verts[other_vert][0];
    edg[1]=verts[this_vert][1]-verts[other_vert][1];
    edg[2]=verts[this_vert][2]-verts[other_vert][2];
      //compute the edge length
    temp_length=edg.length();
      //get the first component
    temp_first = temp_length - lowVal;
    temp_first = fabs(temp_first) - (temp_first);
    temp_first*=temp_first;
      //get the second component
    temp_second = highVal - temp_length;
    temp_second = fabs(temp_second) - (temp_second);
    temp_second*=temp_second;
      //combine the two components
    metric_values[point_counter]=temp_first+temp_second;
      //increment the counter
    ++point_counter;
  }
    //average the metric values of the edges
  fval=average_metrics(metric_values,num_sample_points,err);
    //clean up
  delete[] metric_values;
    //always return true because mesh is always valid wrt this metric.
  return true;
  
}

