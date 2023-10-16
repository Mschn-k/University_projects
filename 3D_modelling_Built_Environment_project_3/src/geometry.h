// Created by Dmitri on 28/03/2022.
//

#ifndef HW03_GEOMETRY_H
#define HW03_GEOMETRY_H

#include <vector>

// CGAL
#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Polyhedron_3.h>
#include <CGAL/Polyhedron_incremental_builder_3.h>
#include <CGAL/Nef_polyhedron_3.h>
#include <CGAL/Convex_hull_traits_3.h>
#include <CGAL/Point_3.h>
typedef CGAL::Exact_predicates_exact_constructions_kernel Kernel;
typedef Kernel::Point_3 Point;
typedef CGAL::Polyhedron_3<Kernel> Polyhedron;
typedef CGAL::Nef_polyhedron_3<Kernel> Nef_polyhedron;



// Vertex
struct Vertex {
    // the coordinates of this vertex:
    Point point;

    // constructor without arguments
    Vertex() : point(Point())
    {}

    // constructor with x,y,z arguments to immediately initialise the point member on this Vertex.
    Vertex(const double &x, const double &y, const double &z) : point(Point(x,y,z))
    {}

    // Store id
    unsigned long id;
};


// Make Vertex "printable" for easy debugging
std::ostream& operator<<(std::ostream& os, const Vertex p) {
    os << "Vertex id: " << p.id << "(" << p.point << ")";
    return os;
}


// === Face ===
struct Face {
    // Points that make up this face, store as vector of pointers to vertices.
    std::vector<Vertex*> vertices;                  // reference by pointer
    std::vector<unsigned long> vertex_indices;      // reference by ID

    // Constructor with 0 arguments
    Face() {
        // ...
    }

    // Constructor with 3 arguments (a triangular face)
    Face(Vertex &v0, Vertex &v1, Vertex &v2) {
        vertices.push_back(&v0);
        vertices.push_back(&v1);
        vertices.push_back(&v2);
    }

    // Constructor with 4 arguments (a face with 4 vertices)
    Face(Vertex &v0, Vertex &v1, Vertex &v2, Vertex &v3) {
        vertices.push_back(&v0);
        vertices.push_back(&v1);
        vertices.push_back(&v2);
        vertices.push_back(&v3);
    }

    // ID
    unsigned long id;
};

// Make face "printable" for easy debugging
std::ostream& operator<<(std::ostream& os, const Face f) {
    os << "Face f" << f.id << ", vertices:";
    for (Vertex* vtx : f.vertices) {
        os << "\t v" <<  " " << *vtx;
    }
    return os;
}

// === Shell ===
struct Shell {
    // Name of the shell (as in .obj file)
    std::string name;

    // Vector of faces
    std::vector<Face*> faces;
    std::vector<unsigned long> face_ids;

    Shell() {}

    Shell(std::string sname) {
        name = sname;
    }

    // ID
    unsigned long id;
};

// Objects
struct Object {
    std::string id;
    std::vector<Shell*> shells;
};

// --
template <class HDS>
struct Polyhedron_builder : public CGAL::Modifier_base<HDS> {
    std::vector<Point> vertices;
    std::vector<std::vector<unsigned long>> faces;

    Polyhedron_builder() {}
    void operator()(HDS& hds) {
        CGAL::Polyhedron_incremental_builder_3<HDS> builder(hds, true);
        std::cout << "building surface with " << vertices.size() << " vertices and " << faces.size() << " faces" << std::endl;

        builder.begin_surface(vertices.size(), faces.size());
        for (auto const &vertex: vertices) builder.add_vertex(vertex);
        for (auto const &face: faces) builder.add_facet(face.begin(), face.end());
        builder.end_surface();
    }
};

struct Shell_explorer {

    // Store vertices
    std::vector<Point> points;
    //std::vector<std::vector<unsigned long>> boundaries;

    // Create vertices subset consisting solely of non-duplicate vertices
    std::unordered_map<std::string, Vertex*> vertex_map;
    std::vector<Vertex*> vertices;

    // store all the boundaries in this shell
    std::vector<std::vector<std::vector<unsigned long>>> boundaries;

    // store the semantics - surface type - of each face
    std::vector<unsigned long> semantics;

    void visit(Nef_polyhedron::Vertex_const_handle v) {}
    void visit(Nef_polyhedron::Halfedge_const_handle he) {}
    void visit(Nef_polyhedron::SHalfedge_const_handle she) {}
    void visit(Nef_polyhedron::SHalfloop_const_handle shl) {}
    void visit(Nef_polyhedron::SFace_const_handle sf) {}

    // index of vertex, used in boundaries, start from 0 in each shell
    //int vertex_index = 0;

    int i = 0; // TODO ?????

    void visit(Nef_polyhedron::Halffacet_const_handle hf) {

        std::cout << vertices.size() << std::endl;
        //std::cout << " - halffacet: " << std::endl;

        std::vector<std::vector<unsigned long>> hface;

        CGAL::Vector_3<Kernel> orth_vect;

        // iterate over each ring of a shell
        for (Nef_polyhedron::Halffacet_cycle_const_iterator it = hf->facet_cycles_begin(); it != hf->facet_cycles_end(); it++)
        {

            //std::cout << " -- ring: ";

            // one face is stored as vector of indices
            std::vector<unsigned long> ring;

            // The orthogonal vector passing through the plane of the face:
            orth_vect = CGAL::orthogonal_vector(hf->plane());

            // Example from https://sympa.inria.fr/sympa/arc/cgal-discuss/2010-02/msg00137.html
            // Access the points in each face
            Nef_polyhedron::SHalfedge_const_handle se = Nef_polyhedron::SHalfedge_const_handle(it);

            // Create an iterator around the facet
            Nef_polyhedron::SHalfedge_around_facet_const_circulator hc_start(se);
            Nef_polyhedron::SHalfedge_around_facet_const_circulator hc_end(hc_start);

            // Iterate overt the facet edge and store the point to the vector
            CGAL_For_all(hc_start,hc_end)
            {
                Nef_polyhedron::SVertex_const_handle svert = hc_start->source();
                Point vpoint = svert->center_vertex()->point();

                // Generate key
                std::stringstream ss_key;
                ss_key << vpoint;
                std::string str_key = ss_key.str();

                unsigned long vertex_index;

                // Only add if no duplicate vertices are found in the vertex map
                if (vertex_map.find(ss_key.str()) == vertex_map.end()) {
                    // No duplicate vertex found
                    vertex_index = vertices.size();

                    // Create new vertex
                    auto vtx = new Vertex();
                    //std::cout << "(new)";
                    vtx->point = vpoint;
                    vtx->id = vertex_index;
                    vertex_map[ss_key.str()] = vtx;

                    // Add to vertices list
                    vertices.push_back(vtx);
                    ring.push_back(vertex_index);
                    //points.push_back(vpoint); // TODO: redundant?

                } else {
                    // Duplicate vertex found, only add found vertex index to boundary
                    //std::cout << "(dup)";
                    vertex_index = vertex_map[str_key]->id; // Looks fishy and wacky, but it's late and my head is like a pot of mashed potatoes... >.<
                    ring.push_back(vertex_index);
                }

                //std::cout << " v" << vertex_index << " [" << vpoint << "]   ";

            }

            // Add ring
            hface.push_back(ring);


        }

        // find the semantic - surface type

        if ((orth_vect[0] == 0) && (orth_vect[1] == 0)) {
            //std::cout << " Horizontal; Ground; 0 "  << std::endl;
            semantics.push_back(0);// ground
        }
        else if (orth_vect[2] == 0) {
            //std::cout << " Vertical; Wall; 1" << std::endl;
            semantics.push_back(1); // wall
        }
        else {
            //std::cout << " Others; Roof; 2" << std::endl;
            semantics.push_back(2); // roof
        }

        // Add face to boundaries
        boundaries.push_back(hface);


        std::cout << std::endl;


        // Trying dmitri's orientation from hw2 - > using plane doesn't work, unless we convert the a b c values of plane to double
        CGAL::Plane_3< Kernel > plane_4 = hf->plane();
        std::vector<Kernel::RT> plane = {plane_4.a(), plane_4.b(), plane_4.c()};

    }
};
#endif //HW03_GEOMETRY_H