// 3D World - City Object Placement Header
// by Frank Gennari
// 08/14/21

#pragma once

#include "city.h"

struct city_obj_t : public sphere_t {
	cube_t bcube;
	city_obj_t() {}
	city_obj_t(point const &pos_, float radius_) : sphere_t(pos_, radius_) {}
	bool operator<(city_obj_t const &b) const {return (bcube.x1() < b.bcube.x1());} // sort by bcube x1
	static void post_draw(draw_state_t &dstate, bool shadow_only) {}
	bool proc_sphere_coll(point &pos_, point const &p_last, float radius_, point const &xlate, vector3d *cnorm) const; // default, can be overridden in derived class
};

struct bench_t : public city_obj_t {
	bool dim, dir;
	bench_t() : dim(0), dir(0) {}
	void calc_bcube();
	static void pre_draw(draw_state_t &dstate, bool shadow_only);
	void draw(draw_state_t &dstate, quad_batch_draw &qbd, float dist_scale, bool shadow_only) const;
};

struct tree_planter_t : public city_obj_t {
	tree_planter_t(point const &pos_, float radius_, float height);
	static void pre_draw(draw_state_t &dstate, bool shadow_only);
	void draw(draw_state_t &dstate, quad_batch_draw &qbd, float dist_scale, bool shadow_only) const;
};

struct fire_hydrant_t : public city_obj_t {
	float cylin_radius;
	vector3d orient;
	fire_hydrant_t(point const &pos_, float radius_, float height, vector3d const &orient_);
	static void pre_draw(draw_state_t &dstate, bool shadow_only);
	static void post_draw(draw_state_t &dstate, bool shadow_only);
	void draw(draw_state_t &dstate, quad_batch_draw &qbd, float dist_scale, bool shadow_only) const;
	bool proc_sphere_coll(point &pos_, point const &p_last, float radius_, point const &xlate, vector3d *cnorm) const;
};

struct divider_t : public city_obj_t {
	unsigned type;
	bool dim, dir;
	uint8_t skip_dims;
	divider_t(cube_t const &c, unsigned type_, bool dim_, bool dir_, unsigned sd=0) :
		city_obj_t(c.get_cube_center(), c.get_bsphere_radius()), type(type_), dim(dim_), dir(dir_), skip_dims(sd) {bcube = c;}
	static void pre_draw(draw_state_t &dstate, bool shadow_only);
	void draw(draw_state_t &dstate, quad_batch_draw &qbd, float dist_scale, bool shadow_only) const;
};

struct swimming_pool_t : public city_obj_t {
	colorRGBA color, wcolor; // wall color and water color
	bool above_ground;

	swimming_pool_t(cube_t const &c, colorRGBA const &color_, colorRGBA const &wcolor_, bool above_ground_) :
		city_obj_t(c.get_cube_center(), c.get_bsphere_radius()), color(color_), wcolor(wcolor_), above_ground(above_ground_) {bcube = c;}
	float get_radius() const {assert(above_ground); return 0.25f*(bcube.dx() + bcube.dy());}
	static void pre_draw(draw_state_t &dstate, bool shadow_only);
	static void post_draw(draw_state_t &dstate, bool shadow_only);
	void draw(draw_state_t &dstate, quad_batch_draw &qbd, float dist_scale, bool shadow_only) const;
	bool proc_sphere_coll(point &pos_, point const &p_last, float radius_, point const &xlate, vector3d *cnorm) const;
};

class city_obj_groups_t : public vector<cube_with_ix_t> {
	map<uint64_t, vector<unsigned> > by_tile;
public:
	template<typename T> void add_obj(T const &obj, vector<T> &objs);
	template<typename T> void create_groups(vector<T> &objs);
};

class city_obj_placer_t {
public: // road network needs access to parking lots and driveways for drawing
	vector<parking_lot_t> parking_lots;
	vector<driveway_t> driveways; // for houses
private:
	vector<bench_t> benches;
	vector<tree_planter_t> planters;
	vector<fire_hydrant_t> fire_hydrants;
	vector<divider_t> dividers; // dividers for residential plots
	vector<swimming_pool_t> pools;
	city_obj_groups_t bench_groups, planter_groups, fire_hydrant_groups, divider_groups, pool_groups; // index is last object in group
	quad_batch_draw qbd;
	vector<city_zone_t> sub_plots; // reused across calls
	unsigned num_spaces, filled_spaces;
	float plot_subdiv_sz;
	
	struct cube_by_x1 {
		bool operator()(cube_t const &a, cube_t const &b) const {return (a.x1() < b.x1());}
	};
	bool gen_parking_lots_for_plot(cube_t plot, vector<car_t> &cars, unsigned city_id, unsigned plot_ix, vect_cube_t &bcubes, vect_cube_t &colliders, rand_gen_t &rgen);
	void add_cars_to_driveways(vector<car_t> &cars, vector<road_plot_t> const &plots, vector<vect_cube_t> &plot_colliders, unsigned city_id, rand_gen_t &rgen);
	void place_trees_in_plot(road_plot_t const &plot, vect_cube_t &blockers, vect_cube_t &colliders, vector<point> &tree_pos, rand_gen_t &rgen, unsigned buildings_end);
	void place_detail_objects(road_plot_t const &plot, vect_cube_t &blockers, vect_cube_t &colliders, vector<point> const &tree_pos, rand_gen_t &rgen, bool is_residential);
	void place_plot_dividers(road_plot_t const &plot, vect_cube_t &blockers, vect_cube_t &colliders, rand_gen_t &rgen);
	void place_residential_plot_objects(road_plot_t const &plot, vect_cube_t &blockers, vect_cube_t &colliders, rand_gen_t &rgen);
	void add_house_driveways(road_plot_t const &plot, vect_cube_t &temp_cubes, rand_gen_t &rgen, unsigned plot_ix);
	template<typename T> void draw_objects(vector<T> const &objs, city_obj_groups_t const &groups,
		draw_state_t &dstate, float dist_scale, bool shadow_only, bool not_using_qbd=0);
public:
	city_obj_placer_t() : num_spaces(0), filled_spaces(0), plot_subdiv_sz(0.0) {}
	void clear();
	void set_plot_subdiv_sz(float sz) {plot_subdiv_sz = sz;}
	void gen_parking_and_place_objects(vector<road_plot_t> &plots, vector<vect_cube_t> &plot_colliders, vector<car_t> &cars, unsigned city_id, bool have_cars, bool is_residential);
	static bool subdivide_plot_for_residential(cube_t const &plot, float plot_subdiv_sz, unsigned parent_plot_ix, vect_city_zone_t &sub_plots);
	void draw_detail_objects(draw_state_t &dstate, bool shadow_only);
	bool proc_sphere_coll(point &pos, point const &p_last, float radius, vector3d *cnorm) const;
	bool line_intersect(point const &p1, point const &p2, float &t) const;
	bool get_color_at_xy(point const &pos, colorRGBA &color, bool skip_in_road) const;
	void get_occluders(pos_dir_up const &pdu, vect_cube_t &occluders) const;
};

inline uint64_t get_tile_id_for_cube(cube_t const &c) {return get_tile_id_containing_point_no_xyoff(c.get_cube_center());}