#include <open3d/Open3D.h>
#include <open3d/geometry/TriangleMesh.h>

#include <Eigen/Core>

#include <string>
#include <vector>
#include <memory>

#include <glog/logging.h>

using TriangleMeshPtr = std::shared_ptr<open3d::geometry::TriangleMesh>;

struct MeshToString
{
    MeshToString(TriangleMeshPtr triangle_mesh);

    size_t vertex_size_{ 0 };
    size_t triangle_size_{ 0 };
    bool has_vertex_color_{ false };
    bool has_triangle_{ false };
    TriangleMeshPtr triangle_mesh_;

    struct Header
    {
                std::string position_string_{"property float x\nproperty float y\nproperty float z\n"};

                std::string color_string_{"property uchar red\nproperty uchar green\nproperty uchar blue\n"};

            std::string vertex_string_{"element vertex "};

            std::string triangle_string_{"element face "};

        std::string file_format_{"ply\n"};
        std::string ply_format_{"format ascii 1.0\n"};
        std::string list_string_{"property list uchar uint vertex_indices\n"};
        std::string end_header_{"end_header\n"};

        std::string header_string_{""};

        Header() {}
        Header(TriangleMeshPtr triangle_mesh)  { }
    };      // struct Header

    struct VertexPropertyList
    {
        VertexPropertyList() {};
        VertexPropertyList(const std::vector<Eigen::Vector3d> vertices);
        VertexPropertyList(const std::vector<Eigen::Vector3d> vertices, std::vector<Eigen::Vector3d> colors);
        // VertexPropertyList(const TriangleMeshPtr triangle_mesh);

        std::string generateString();

        std::vector<std::string> list_;
        std::vector<Eigen::Vector3d> vertices_;
        std::vector<Eigen::Vector3d> vertex_colors_;     // TODO Need to convert to int
        std::string string_buffer_;
    };

    struct TrianglePropertyList
    {
        TrianglePropertyList() {};
        TrianglePropertyList(std::vector<Eigen::Vector3i> vertex_indices);

        std::vector<std::string> list_;
        std::vector<Eigen::Vector3i> vertex_indices_;
        std::string generateString();
        std::string string_buffer_;
    };

    std::string generateHeaderString();
    std::string generateFileString();

    // TODO Add apostrophe at first and in the end of the string
    // The apostrophe is for WKT string
    std::string apostrophe_{"'"};
    Header header_;
    VertexPropertyList vertex_property_list_;
    TrianglePropertyList triangle_property_list_;
    std::string file_string_{""};
};      // struct MeshToString
