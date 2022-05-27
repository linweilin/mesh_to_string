#include <open3d/Open3D.h>
#include <glog/logging.h>

#include "mesh_to_string_buffer.h"


int main(int argc, char * argv[])
{
    
    LOG(INFO) << "Hello, test_mesh_to_string_buffer" << std::endl;

    auto mesh = open3d::io::CreateMeshFromFile(argv[1]);

    if(mesh == nullptr) {

        LOG(FATAL) << "Failed to read mesh from file " << argv[1];
    }

    MeshToString mesh_to_string(mesh);
    auto mesh_string = mesh_to_string.generateFileString();
    LOG(INFO) << "\n" << mesh_string;

    return 0;
}
