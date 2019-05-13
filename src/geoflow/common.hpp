// This file is part of Geoflow
// Copyright (C) 2018-2019  Ravi Peters, 3D geoinformation TU Delft

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#pragma once

#include <algorithm>
#include <vector>
#include <array>
#include <optional>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace geoflow {

  // enum TerminalType : uint32_t{
  //   TT_any = 0,
  //   TT_float,
  //   TT_int,
  //   TT_vec1ui,
  //   TT_vec1i,
  //   TT_vec1f,
  //   TT_vec1b,
  //   TT_vec2f,
  //   TT_vec3f,
  //   TT_vec6f,
  //   TT_vec_float,
  //   TT_colmap,
  //   TT_geometry,
  //   TT_feature,
  //   TT_point_collection, 
  //   TT_point_collection_list, 
  //   TT_triangle_collection,
  //   TT_segment_collection,
  //   TT_segment_collection_list,
  //   TT_line_string_collection,
  //   TT_linear_ring,
  //   TT_linear_ring_collection,
  //   TT_attribute_map_f
  // };

typedef std::array<float,3> arr3f;
typedef std::vector<arr3f> vec3f;
typedef std::vector<std::array<float,2>> vec2f;
typedef std::vector<int> vec1i;
typedef std::vector<bool> vec1b;
typedef std::vector<float> vec1f;
typedef std::vector<size_t> vec1ui;
typedef std::vector<std::string> vec1s;

typedef std::unordered_map< std::string, std::vector<float>> AttributeMap;

class Box {
  private:
  std::array<float,3> pmin, pmax;
  bool just_cleared;
  public:
  Box(){
      clear();
  }

	std::array<float, 3> min() const {
		return pmin;
	}
	std::array<float, 3> max() const {
		return pmax;
	}
  void set(std::array<float,3> nmin, std::array<float,3> nmax) {
    pmin = nmin;
    pmax = nmax;
    just_cleared = false;
  }
  void add(float p[]){
      if(just_cleared){
          pmin[0] = p[0];
          pmin[1] = p[1];
          pmin[2] = p[2];
          pmax[0] = p[0];
          pmax[1] = p[1];
          pmax[2] = p[2];
          just_cleared = false;
      }
      pmin[0] = std::min(p[0], pmin[0]);
      pmin[1] = std::min(p[1], pmin[1]);
      pmin[2] = std::min(p[2], pmin[2]);
      pmax[0] = std::max(p[0], pmax[0]);
      pmax[1] = std::max(p[1], pmax[1]);
      pmax[2] = std::max(p[2], pmax[2]);
  }
  void add(arr3f a){
    add(a.data());
  }
  void add(const Box& otherBox){
      add(otherBox.min());
      add(otherBox.max());
  }
  void add(Box& otherBox){
      add(otherBox.min());
      add(otherBox.max());
  }
  void add(vec3f& vec){
      for (auto& p : vec)
        add(p);
  }
  void clear(){
      pmin.fill(0);
      pmax.fill(0);
      just_cleared = true;
  }
  bool isEmpty() const {
      return just_cleared;
  }
  glm::vec3 center() const {
      return {(pmax[0]+pmin[0])/2, (pmax[1]+pmin[1])/2, (pmax[2]+pmin[2])/2};
  }
};

template<typename geom_def> class GeometryCollection : public std::vector<geom_def> {
  protected:
  std::optional<Box> bbox;
  virtual void compute_box() =0;

  public:
  virtual size_t vertex_count() const=0;
  virtual Box& box() {
    if (!bbox.has_value()){
      compute_box();
    }
    return *bbox;
  };
  size_t dimension() {
    return 3;
  }
};

// geometry types:
// typedef arr3f Point;
typedef std::array<arr3f, 3> Triangle;
typedef std::array<arr3f, 2> Segment;
// typedef vec3f LineString;
// typedef vec3f LinearRing;
class Arr3fCollection:public GeometryCollection<arr3f> {
  public:
  size_t vertex_count() const{
    return size();
  }
  virtual void compute_box() {
    if (!bbox.has_value()) {
      bbox=Box();
      bbox->add(*this);
    }
  }
};
class PointCollection:public Arr3fCollection {};
class LineString:public Arr3fCollection {};
class LinearRing:public Arr3fCollection {};

class TriangleCollection:public GeometryCollection<Triangle> {
  public:
  size_t vertex_count() const {
    return size()*3;
  }
  virtual void compute_box() {
    if (!bbox.has_value()) {
      bbox=Box();
      for(auto& t : *this){
        bbox->add(t[0]);
        bbox->add(t[1]);
        bbox->add(t[2]);
      }
    }
  }
};
class SegmentCollection:public GeometryCollection<Segment> {
  public:
  size_t vertex_count() const {
    return size()*2;
  }
  virtual void compute_box() {
    if (!bbox.has_value()) {
      bbox=Box();
      for(auto& t : *this){
        bbox->add(t[0]);
        bbox->add(t[1]);
      }
    }
  }
};
// typedef GeometryCollection<arr3f, point> PointCollection;
class LineStringCollection:public GeometryCollection<LineString> {
  public:
  size_t vertex_count() const{
    size_t result=0;
    for (auto& vec : *this) {
      result += vec.size();
    }
    return result;
  }
  virtual void compute_box() {
    if (!bbox.has_value()) {
      bbox=Box();
      for(auto& vec : *this){
        bbox->add(vec);
      }
    }
  }
};
class LinearRingCollection:public GeometryCollection<LinearRing> {
  size_t vertex_count() const{
    size_t result=0;
    for (auto& vec : *this) {
      result += vec.size();
    }
    return result;
  }
  virtual void compute_box() {
    if (!bbox.has_value()) {
      bbox=Box();
      for(auto& vec : *this){
        bbox->add(vec);
      }
    }
  }
};

}
