/**
 * GEO1004 - HW03
 *
 * Cynthia Cai
 * Katrin Meschin
 * Dmitri Visser
 *
 */


// Default includes
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <unordered_map>
#include <string>

// Include geometry header (vertices, faces, shells, etc...)
#include "geometry.h"
#include <CGAL/Nef_polyhedron_3.h>
#include <CGAL/convex_hull_3.h>
#include <CGAL/Point_3.h>

// Conversion to mesh
#include <CGAL/Surface_mesh.h>
#include <CGAL/boost/graph/convert_nef_polyhedron_to_polygon_mesh.h>

// JSON
#include "json.hpp"
using json = nlohmann::json;
// File IO
#include <CGAL/IO/Nef_polyhedron_iostream_3.h>
#include <CGAL/OFF_to_nef_3.h>

typedef Nef_polyhedron::Volume_const_iterator Volume_const_iterator;
typedef Nef_polyhedron::Shell_entry_const_iterator Shell_entry_const_iterator;

typedef CGAL::Exact_predicates_exact_constructions_kernel Exact_kernel;
typedef CGAL::Surface_mesh<Exact_kernel::Point_3> Surface_mesh;

void parse_args(int argc, const char *argv[], std::string &infile, std::string &outfile, std::string &dir, bool &write_off);

// IO
bool read_obj(const std::string& filepath, std::vector<Vertex>& vertices, std::vector<Face>& faces, std::vector<Shell>& shells);
void write_nef_polyhedron(std::string filepath, Nef_polyhedron& nef_polyhedron);
void write_polyhedron(std::string filepath, Polyhedron& polyhedron);

void link_pointers(std::vector<Vertex>& vertices, std::vector<Face>& faces, std::vector<Shell>& shells);
Polyhedron construct_polyhedron(const Shell &shell);
std::vector<Vertex>create_map(std::vector<Vertex>& vertices);
Nef_polyhedron create_nef(Polyhedron& polyhedron);

void construct_json(json &cj, Nef_polyhedron &nef);

int main(int argc, const char * argv[]) {

    // Input and output files
    std::string infile = "openhouse.obj";    // default .obj file
    //std::string infile = "testhouse.obj";    // default .obj file
    std::string outfile = "output.json";  // default .json file
    std::string dir = "../../data/";    // default directory
    bool write_off = false;

    // Ask for user input to specify input file (can override the defaults)
    parse_args(argc, argv, infile, outfile, dir, write_off);

    // === From obj to NEF polyhedra ===
    // Store faces and vertices
    std::vector<Vertex> vertices;
    std::vector<Face> faces;
    std::vector<Shell> shells;

    // Load faces and vertices
    try {
        read_obj(infile, vertices, faces, shells);
    } catch (std::string err) {
        std::cout << "An error occurred when reading " << infile << ": " << err << std::endl;
        return -1;
    }


    // Link pointers
    link_pointers(vertices, faces, shells);

    // Store polyhedrons
    std::vector<Polyhedron> polyhedrons;

    // Construct polyhedrons
    for (auto const &shell : shells) {
        Polyhedron polyhedron = construct_polyhedron(shell);
        polyhedrons.push_back(polyhedron);
    }

    int i = 0;
    std::string pathout;

    // Write out polyhedrons
    if (write_off) {
        for (auto &polyhedron: polyhedrons) {
            pathout = dir + "polyhedron_" + std::to_string(i) + ".off";
            write_polyhedron(pathout, polyhedron);
            i++;
        }
    }

    // construct nef polyhedra
    std::vector<Nef_polyhedron> nef_polyhedrons;
    for (auto &polyhedron : polyhedrons){
        Nef_polyhedron nef = create_nef(polyhedron);
        nef_polyhedrons.push_back(nef);
    }
    std::cout << "Number of NEF polyhedra: " << nef_polyhedrons.size() << std::endl;

    std::cout << nef_polyhedrons[0].number_of_edges();



    // Write out nefpolyhedrons
    if (write_off) {
        i = 0;
        for (auto &nef_polyhedron: nef_polyhedrons) {
            pathout = dir + "nef_polyhedron_" + std::to_string(i) + ".off";
            write_nef_polyhedron(pathout, nef_polyhedron);
            i++;
        }
    }

    // Create single big Nef_polyhedron
    Nef_polyhedron bignef;
    std::cout << "Creating single big nef_polyhedron..." << std::endl;
    i = 0;
    for (auto &nef_polyhedron : nef_polyhedrons) {
        if (nef_polyhedron.is_valid()) {
            std::cout << " - adding nef_polyhedron " << i << std::endl;
            bignef = bignef + nef_polyhedron;
        } else {
            std::cout << " - could not add nef_polyhedron " << i << ": INVALID NEF" << std::endl;
        }
        i++;
    }

    std::cout << "\n\nBig_Nef created, stats:\n"
              << " -- Number of vertices:  " << bignef.number_of_vertices() << "\n"
              << " -- Number of facets:    " << bignef.number_of_facets() << "\n"
              << " -- Number of halfedges: " << bignef.number_of_halfedges() << "\n"
              << " -- is_valid:            " << bignef.is_valid() << "\n"
              << " -- is_simple:           " << bignef.is_simple() << std::endl
              << " -- Number of volumes:   " << bignef.number_of_volumes() << "\n"<< std::endl;

    if (write_off) {
        write_nef_polyhedron("../../data/export/bignefout.off", bignef);
    }

    // Write to JSON
    json cj;

    construct_json(cj, bignef);

    std::string json_string = cj.dump(2);
    std::ofstream out_stream(outfile);
    out_stream << json_string;
    out_stream.close();

    return 0;
}

/*
 * Function to parse the user input
 *
 * Input:
 *  & infile    (reference) input path
 *  & outfile   (reference) output path
 *  & dir       (reference) dir (with trailing slash)
 *
 * Output:
 *  void
 */
void parse_args(int argc, const char *argv[], std::string &infile, std::string &outfile, std::string &dir, bool &write_off) {

    if (argc == 1) {
        // User did not specify anything
        std::cout << "No input arguments provided. Using defaults.\n";
    }

    if (argc == 2) {
        // User specified only [dir]
        dir = argv[1];
        std::cout << "Directory provided. Using:\n";
    }

    if (argc > 2) {
        // User specified both [input] and [output]
        dir = "";
        infile = argv[1];
        outfile = argv[2];
    }

    if (argc > 3) {
        // User specified both [input] and [output]
        dir = "";
        infile = argv[1];
        outfile = argv[2];
        if (argv[3] == "--write-off") {
            write_off = true;
            std::cout << "Verbose writing of .off files = TRUE" << std::endl;
        }
    }

    infile = dir + infile;
    outfile = dir + outfile;
    std::cout << "Input:  " << infile << "\n"
              << "Output: " << outfile << "\n" << std::endl;

}


/*
 * Function that reads the obj file and stores the vertices
 *
 * Input:
 *  & filepath  (reference) Input file
 *  & vertices  (reference) vector of vertices
 *  & faces     (reference) vector of faces
 *  & shells    (reference) vector of shells
 *
 * Output:
 *  bool        success
 *
 */
bool read_obj(const std::string& filepath, std::vector<Vertex>& vertices, std::vector<Face>& faces, std::vector<Shell>& shells) {
    std::cout << "Reading file: " << filepath << std::endl;

    // Create input stream class and open file in read mode
    std::ifstream stream_in(filepath.c_str(), std::ifstream::in);
    if (!stream_in) {
        std::string str = "Input file not found or file could not be opened.";
        throw str;
    }

    // Keep track of integer vertex indices
    unsigned long vertex_id = 0;
    unsigned long face_id = 0;
    unsigned long shell_id = 0;


    // File is opened, read file
    std::string line;
    while (std::getline(stream_in, line)) {
        std::istringstream iss(line);
        std::string word;
        iss >> word;

        // Handle lines starting with g (= groups / shells)
        if (word == "g") {
            std::string shell_name;
            while (iss >> word) shell_name = word; // Read shell name (eg: product-c85b92af-3024-476e-b778-c01d403d33ea-body)
            shells.emplace_back(shell_name);

            // Assign shell id
            shells.back().id = shell_id;
            shell_id++;
        }

        // Handle lines starting with v (= vertices)
        if (word == "v") {
            std::vector<float> coordinates;

            // Parse line
            while (iss >> word) coordinates.push_back(std::stof(word));
            if (coordinates.size() == 3) vertices.emplace_back(coordinates[0], coordinates[1], coordinates[2]);
            else vertices.push_back(Vertex());

            // Assign vertex id
            vertices.back().id = vertex_id;
            vertex_id++;
        }

        // Handle lines starting with f (= faces)
        if (word == "f") {
            // Store pointers that point to the actual vertices
            std::vector<unsigned long> vertex_indices;

            // Parse line
            while (iss >> word) {
                // Read vertex index (an integer) with std::stoi
                // Notice that obj files start counting at 1, whereas our vector starts at 0. So, subtract 1 to correctly refer to the vertex.
                int vertex_index = std::stoi(word) - 1;

                // Check whether we can actually access this vertex
                if (vertex_index >= vertices.size()) {
                    std::cerr << "Found face with vertex reference = " << vertex_index << ", which is out of bounds." << std::endl;
                    continue;
                }

                // Convert vertex index to actual vertex and store the reference
                vertex_indices.push_back(vertex_index);                     // reference by ID
            }

            // Add new face to vector
            faces.emplace_back();
            //faces.back().vertices = vertex_pointers;          // set last face vertices (by pointer)
            faces.back().vertex_indices = vertex_indices;       // set last face vertices (by ID)

            // Assign face id
            faces.back().id = face_id;
            face_id++;

            // Add face to shell
            shells.back().face_ids.push_back(face_id-1);

        }

    }

    std::cout << "Number of shells read: " << shells.size() << std::endl;
    std::cout << "Number of vertices read: " << vertices.size() << std::endl;
    std::cout << "Number of faces read:    " << faces.size() << "\n" << std::endl;

    return true;

}

/*
 * Function that relinks all pointers based on the unsigned long indices.
 *
 * Input:
 *  & vertices  (reference) vector of vertices
 *  & faces     (reference) vector of faces
 *  & shells    (reference) vector of shells
 *
 */
void link_pointers(std::vector<Vertex>& vertices, std::vector<Face>& faces, std::vector<Shell>& shells) {

    std::cout << "Relinking pointers" << std::endl;

    // Link pointer in faces to vertices
    for (auto &face : faces) {
        for (auto vertex_id : face.vertex_indices) {
            face.vertices.push_back(&vertices[vertex_id]);
        }
    }

    // Link pointer in shells to faces
    for (auto &shell : shells) {
        for (auto face_id : shell.face_ids) {
            shell.faces.push_back(&faces[face_id]);
        }
    }

}

/*
 * Create unordered map from the vertices to get rid of duplicates
 * Then adds those vertices to a new vector, returns the vector
 *
 * Input:
 * & vertices list
 *
 * Output
 * unordered map
 * */
std::vector<Vertex> create_map(std::vector<Vertex>& vertices){
    std::unordered_map <std::string, Vertex> vert_map;
    for (auto &vt : vertices) {
        std::stringstream ss;
        ss << vt.point.x();
        ss<<vt.point.y();
        ss<<vt.point.z();
        std::string key = ss.str();
        // to check, print out the key. Look like -4.810.0451.91
        //std::cout << vt <<' ' << key << std::endl;
        if (vert_map.find(key) == vert_map.end()){
            vert_map[key] = vt;
        }
    }
    // from unordered map, create a new vertices vector
    std::vector<Vertex> vertices_cleaned;
    std::unordered_map<std::string, Vertex>:: iterator itr;

    for (itr = vert_map.begin(); itr != vert_map.end(); itr++){
        vertices_cleaned.push_back((itr->second));
    }
    return(vertices_cleaned);
}

/*
 * Construct a polyhedron and return it
 *
 * Input:
 *  & shell  (reference) shell
 *
 * Output:
 *  Polyhedron
 *
 */
Polyhedron construct_polyhedron(const Shell &shell) {

    std::cout << "Constructing shell " << shell.name << ", number of faces: " << shell.faces.size() << std::endl;


    // Create instance of polyhedron
    Polyhedron polyhedron;
    Polyhedron_builder<Polyhedron::HalfedgeDS> polyhedron_builder;


    // Create vertices subset consisting solely of non-duplicate vertices
    std::map<unsigned long, Vertex*> vert_map;
    unsigned long num_vtx = 0;
    for (auto const &face : shell.faces) {
        for (auto const &vertex : face -> vertices) {
            vert_map[vertex->id] = vertex;
            num_vtx++;
        }
    }
    std::cout << "Num_vtx: " << num_vtx << ", reduced: " << vert_map.size() << std::endl;

    // Add non-duplicate vertices subset to polyhedron_builder
    for (auto const &unique_vertex : vert_map) {
        polyhedron_builder.vertices.push_back(unique_vertex.second->point);
    }

    // Vertex ID for polyhedron builder
    unsigned long vtxid = 0;

    for (auto const &face: shell.faces) {
        std::cout << " - adding facet with " << face->vertices.size() << " vertices: ";

        // Add empty facet
        polyhedron_builder.faces.emplace_back();
        for (auto const &vertex: face->vertices) {

            // Add non duplicate index (find in map)
            vtxid = (unsigned long) std::distance(vert_map.begin(), vert_map.find(vertex->id));
            polyhedron_builder.faces.back().push_back(vtxid);

            // Output (polyhedron_vertex_id) / (our_vertex_id) : [ vertex_coordinates ]
            std::cout << " v" << vtxid << "/" << vertex->id << ": [" << vertex->point << "]    ";

        }

        std::cout << std::endl;

    }

    polyhedron.delegate(polyhedron_builder);

    std::cout << "Polyhedron created, stats:\n"
              << " - Number of vertices:  " << polyhedron.size_of_vertices() << "\n"
              << " - Number of facets:    " << polyhedron.size_of_facets() << "\n"
              << " - Number of halfedges: " << polyhedron.size_of_halfedges() << "\n"
              << " - is_closed:           " << polyhedron.is_closed() << "\n"
              << " - is_valid:            " << polyhedron.is_valid() << "\n"<< std::endl;

    return polyhedron;
}


Nef_polyhedron create_nef(Polyhedron &polyhedron){

    std::cout << "Converting polyhedron to Nef_polyhedron..." << std::endl;

    // Check if polyhedron closed and valid
    if(polyhedron.is_closed() && polyhedron.is_valid()) {
        std::cout << " - closed and valid " << std::endl;
    }
    else{
        if(polyhedron.is_closed() == false){
            std::cout << " - polyhedron not closed, creating a convex hull" << std::endl;
            CGAL::convex_hull_3(polyhedron.points_begin(), polyhedron.points_end(), polyhedron);

        }

        if(polyhedron.is_valid() == false){
            // Notify of invalid polyhedron
            std::cout << "\n THIS POLYHEDRON IS NOT VALID\n";
        }
    }

    Nef_polyhedron nef_3(polyhedron);

    // check if the created Nef polyhedron is valid
    if(nef_3.is_valid() == false){
        Polyhedron convexhull;
        CGAL::convex_hull_3(polyhedron.points_begin(), polyhedron.points_end(), convexhull);
        Nef_polyhedron newnef = Nef_polyhedron(convexhull);
        // assign the nef polyhedron created from the convex hull of polyhedron as the nef_3
        nef_3 = newnef;
    }


    std::cout << " - Nef_3 created, stats:\n"
              << " -- Number of vertices:  " << nef_3.number_of_vertices() << "\n"
              << " -- Number of facets:    " << nef_3.number_of_facets() << "\n"
              << " -- Number of halfedges: " << nef_3.number_of_halfedges() << "\n"
              << " -- is_valid:            " << nef_3.is_valid() << "\n"<< std::endl;


    return nef_3;
}


void write_polyhedron(std::string filepath, Polyhedron& polyhedron) {

    std::cout << "--- Writing polyhedron ---" << std::endl;
    std::cout << "# Output file path: " << filepath << std::endl;

    std::ofstream outfile("" + filepath,std::ofstream::out);
    if (outfile.is_open()) {

        outfile << polyhedron;

        outfile.close();

        std::cout << "# Finished writing." << std::endl;
    } else {
        std::cout << "## Unable to open file!" << std::endl;
    }
}

void write_nef_polyhedron(std::string filepath, Nef_polyhedron& nef_polyhedron) {

    std::cout << "--- Writing nef_polyhedron ---" << std::endl;
    std::cout << "# Output file path: " << filepath << std::endl;

    std::ofstream outfile("" + filepath,std::ofstream::out);
    if (outfile.is_open()) {

        Surface_mesh output;
        CGAL::convert_nef_polyhedron_to_polygon_mesh(nef_polyhedron, output);


        outfile << output;

        outfile.close();

        std::cout << "# Finished writing." << std::endl;
    } else {
        std::cout << "## Unable to open file!" << std::endl;
    }
}


void construct_json(json &cj, Nef_polyhedron &nef) {

    // Add header information
    cj["type"] = "CityJSON";
    cj["version"] = "1.1";
    cj["transform"] = json::object();
    cj["transform"]["scale"] = json::array({1.0, 1.0, 1.0});
    cj["transform"]["translate"] = json::array({0.0, 0.0, 0.0});

    std::string cityobject_namebase = "OpenHouse";

    // Add city objects
    cj["CityObjects"] = nlohmann::json::object();
    auto &co = cj["CityObjects"];

    // Initialise list of vertices
    cj["vertices"] = json::array();

    // Keep track of shells
    int shell = 0;

    // Go through volumes
    int i = 0;
    Nef_polyhedron::Volume_const_iterator current_volume;
    CGAL_forall_volumes(current_volume, nef) {

        // If i == 0, it is the first volume, the 'universe'. In this we only create the city object
        if (i == 0) {
            std::cout << "\n\nVolume " << i << std::endl;
            //cj["CityObjects"] = json::object();

            // Add building
            co[cityobject_namebase]["type"] = "Building";
            auto &bldg = co[cityobject_namebase];

            bldg["geometry"] = json::array();
            bldg["attributes"] = json::object();
            bldg["geometry"] = json::array();
            bldg["children"] = json::array();

            i++;
            shell++;
        }

        // if i == 1, then it is the second volume which has 2 shells
        if(i==1){
            std::string child = cityobject_namebase + '_' + std::to_string(i);
            cj["CityObjects"][child]["type"] = "BuildingPart";

            cj["CityObjects"][child]["parents"] = json::array();
            cj["CityObjects"][child]["parents"][0] = cityobject_namebase;
            cj["CityObjects"][child]["attributes"] = json::object();

            // Add the child's name to the parent cityobject
            cj["CityObjects"][cityobject_namebase]["children"].push_back(child);

            // inside geometry
            cj["CityObjects"][child]["geometry"] = json::array();
            cj["CityObjects"][child]["geometry"][0]["lod"] = "2.2";
            cj["CityObjects"][child]["geometry"][0]["type"] = "Solid";
            cj["CityObjects"][child]["geometry"][0]["boundaries"] = json::array();

            // inside semantics
            cj["CityObjects"][child]["geometry"][0]["semantics"] = json::object();
            auto &semantics = cj["CityObjects"][child]["geometry"][0]["semantics"];
            semantics["surfaces"] = json::array();
            semantics["surfaces"][0]["type"] = "GroundSurface";
            semantics["surfaces"][1]["type"] = "WallSurface";
            semantics["surfaces"][2]["type"] = "RoofSurface";
            semantics["values"] = json::array();

            // Go through shells
            Nef_polyhedron::Shell_entry_const_iterator current_shell;
            CGAL_forall_shells_of(current_shell, current_volume) {
                std::cout << "Shell " << shell << std::endl;


                // Create shell explorer
                Shell_explorer se;
                Nef_polyhedron::SFace_const_handle sface_in_shell(current_shell);
                nef.visit_shell_objects(sface_in_shell, se);
                //std::vector <std::vector<int>> boundaries = se.boundaries;

                cj["CityObjects"][child]["geometry"][0]["semantics"]["values"][0] = se.semantics;
                cj["CityObjects"][child]["geometry"][0]["boundaries"] = json::array();
                cj["CityObjects"][child]["geometry"][0]["boundaries"][0] = se.boundaries;

                // Add list of vertices to JSON
                for (auto vtx : se.vertices) {
                    //std::cout << "adding to json: " << vtx->point << std::endl;
                    json vtx_coordinates = {
                            CGAL::to_double(vtx->point.x()),
                            CGAL::to_double(vtx->point.y()),
                            CGAL::to_double(vtx->point.z())
                    };

                    cj["vertices"].push_back(vtx_coordinates);
                }

                shell++;
                i++;

            }

        }

        // the volumes from 2 and up are considered Building rooms, each having 1 shell. Create a city object for each
        if(i>=2){
            Nef_polyhedron::Shell_entry_const_iterator current_shell;
            // each volume from 2 to up has only one shell
            CGAL_forall_shells_of(current_shell, current_volume) {


                std::string room = cityobject_namebase + '_' + std::to_string(i)+ '_' + std::to_string(shell);
                cj["CityObjects"][room]["type"] = "BuildingRoom";
                cj["CityObjects"][room]["parents"] = cityobject_namebase;
                cj["CityObjects"][room]["attributes"] = json::object();

                // Add the child's name to the parent cityobject
                cj["CityObjects"][cityobject_namebase]["children"].push_back(room);

                // inside geometry
                cj["CityObjects"][room]["geometry"] = json::array();

                //access shell explorer
                Shell_explorer se;
                Nef_polyhedron::SFace_const_handle sface_in_shell(current_shell);
                nef.visit_shell_objects(sface_in_shell, se);
                //std::vector <std::vector<int>> boundaries = se.boundaries;

                cj["CityObjects"][room]["geometry"][0]["boundaries"] = se.boundaries;

                // add vertices place for shell's
                //cj["CityObjects"]["vertices"] = json::array();
                shell++;
                i++;
            }
        }

    }



}