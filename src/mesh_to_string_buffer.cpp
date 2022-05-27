#include "mesh_to_string_buffer.h"

MeshToString::MeshToString(TriangleMeshPtr triangle_mesh)
    : triangle_mesh_(std::make_shared<open3d::geometry::TriangleMesh>(*(triangle_mesh.get()))), vertex_size_(triangle_mesh->vertices_.size()), has_vertex_color_(triangle_mesh->HasVertexColors()), triangle_size_(triangle_mesh->triangles_.size()), vertex_property_list_(triangle_mesh->vertices_, triangle_mesh->vertex_colors_), triangle_property_list_(triangle_mesh->triangles_)
{
    if(triangle_size_ != 0) {

        has_triangle_ = true;
    }

    vertex_property_list_.vertices_ = triangle_mesh_->vertices_;
    if(has_vertex_color_) {

        vertex_property_list_.vertex_colors_ = triangle_mesh_->vertex_colors_;
    }
    if(has_triangle_) {

        triangle_property_list_.vertex_indices_ = triangle_mesh_->triangles_;
    }
}

std::string MeshToString::generateHeaderString()
{
    header_.header_string_ = apostrophe_
                           + header_.file_format_ 
                           + header_.ply_format_
                           + header_.vertex_string_ + std::to_string(vertex_size_) + "\n"
                           + header_.position_string_;

    if(has_vertex_color_) {

        header_.header_string_ += header_.color_string_;
    }

    if(has_triangle_) {

        header_.header_string_ += header_.triangle_string_ + std::to_string(triangle_size_) + "\n";
    }

    header_.header_string_ += header_.list_string_ + header_.end_header_;

    return header_.header_string_;
}

std::string MeshToString::generateFileString()
{
    auto header_string = generateHeaderString();
    auto vertex_property_list_string = vertex_property_list_.generateString();
    auto triangle_property_list_string = triangle_property_list_.generateString();

    file_string_ = header_string + vertex_property_list_string + triangle_property_list_string + apostrophe_;

    return file_string_;
}

MeshToString::VertexPropertyList::VertexPropertyList(const std::vector<Eigen::Vector3d> vertices)
    : vertices_(vertices)
{
    list_.reserve(vertices_.size());
}

MeshToString::VertexPropertyList::VertexPropertyList(const std::vector<Eigen::Vector3d> vertices, std::vector<Eigen::Vector3d> vertex_colors)
    : vertices_(vertices), vertex_colors_(vertex_colors)
{
    if(vertices_.size() != vertex_colors_.size()) {

        LOG(WARNING) << "vertices size is not equal to vertex colors size!";
    }
    list_.reserve(vertices_.size());
}

std::string MeshToString::VertexPropertyList::generateString()
{
    // TODO reserve the size of string_buffer_;
    size_t string_buffer_size = 0;
    if(!vertex_colors_.empty()) {

        for(size_t i = 0; i < vertices_.size(); ++i) {

            auto temp_string = std::to_string(vertices_.at(i)(0)) + " "
                             + std::to_string(vertices_.at(i)(1)) + " "
                             + std::to_string(vertices_.at(i)(2)) + " "
                             + std::to_string(static_cast<int>(vertex_colors_.at(i)(0))) + " "
                             + std::to_string(static_cast<int>(vertex_colors_.at(i)(1))) + " "
                             + std::to_string(static_cast<int>(vertex_colors_.at(i)(2))) + "\n";
            list_.push_back(std::move(temp_string));

            string_buffer_size += sizeof(list_.at(i));      // 粗略计算每一行的buffer大小
        }
    }
    else {

        for(size_t i = 0; i < vertices_.size(); ++i) {

            auto temp_string = std::to_string(vertices_.at(i)(0)) + " "
                             + std::to_string(vertices_.at(i)(1)) + " "
                             + std::to_string(vertices_.at(i)(2)) + "\n";
            list_.push_back(std::move(temp_string));

            string_buffer_size += sizeof(list_.at(i));      // 粗略计算每一行的buffer大小
        }
    }
    string_buffer_.reserve(string_buffer_size);
    for(auto &b : list_) {

        string_buffer_ += b;
    }

    return string_buffer_;
}

MeshToString::TrianglePropertyList::TrianglePropertyList(const std::vector<Eigen::Vector3i> vertex_indices) : vertex_indices_(vertex_indices)
{
    list_.reserve(vertex_indices_.size());
}

std::string MeshToString::TrianglePropertyList:: generateString()
{
    // TODO reserve the size of string_buffer_;
    size_t string_buffer_size = 0;
    for(size_t i = 0; i < vertex_indices_.size(); ++i) {

        auto temp_string = "3 " + std::to_string(vertex_indices_.at(i)(0)) + " "
                                + std::to_string(vertex_indices_.at(i)(1)) + " "
                                + std::to_string(vertex_indices_.at(i)(2)) + "\n";
        list_.push_back(std::move(temp_string));

        string_buffer_size += sizeof(list_.at(i));      // 粗略计算每一行的buffer大小
    }
    string_buffer_.reserve(string_buffer_size);
    for(auto &b : list_) {

        string_buffer_ += b;
    }

    return string_buffer_;
}
