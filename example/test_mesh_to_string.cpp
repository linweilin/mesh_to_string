#include <open3d/Open3D.h>
#include <glog/logging.h>

#include "mesh_to_string.h"


int main(int argc, char * argv[])
{
    
    LOG(INFO) << "Hello, test_mesh_to_string" << std::endl;

    auto mesh = open3d::io::CreateMeshFromFile(argv[1]);

    if(mesh == nullptr) {

        LOG(FATAL) << "Failed to read mesh from file " << argv[1];
    }

    MeshToString mesh_to_string(mesh);
    auto mesh_string = mesh_to_string.generateFileString();
    // LOG(INFO) << "\n" << mesh_string;
    

    auto mesh_from_string = mesh_to_string.generateMeshFromString(mesh_string);
    if(mesh_from_string == nullptr) {

        LOG(FATAL) << "mesh_from_string is nullptr.";
    }
    LOG(INFO) << "result mesh:\n"
        << "vertex size = " << mesh_from_string->vertices_.size() << "\n"
        << "vertex color size = " << mesh_from_string->vertex_colors_.size() << "\n"
        << "triangle size = " << mesh_from_string->triangles_.size();

    open3d::visualization::DrawGeometries({mesh_from_string}, "test mesh_from_string");

    return 0;
}
