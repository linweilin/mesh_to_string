#include "mesh_to_string.h"

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
                           + header_.comment_       // keep format uniformed
                           + header_.vertex_string_ + std::to_string(vertex_size_) + "\n"
                           + header_.position_string_;

    if(has_vertex_color_) {

        header_.header_string_ += header_.color_string_;
    }

    // Need to add this even when there is no triangle
    header_.header_string_ += header_.triangle_string_ + std::to_string(triangle_size_) + "\n";

    header_.header_string_ += header_.list_string_ + header_.end_header_ + "\n";

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

            std::stringstream temp_string_stream;
            temp_string_stream << vertices_.at(i)(0) << " "
                               << vertices_.at(i)(1) << " "
                               << vertices_.at(i)(2) << " "
                               << static_cast<int>(vertex_colors_.at(i)(0)) << " "
                               << static_cast<int>(vertex_colors_.at(i)(1)) << " "
                               << static_cast<int>(vertex_colors_.at(i)(2)) << "\n";

            list_.push_back(std::move(temp_string_stream.str()));

            string_buffer_size += sizeof(list_.at(i));      // 粗略计算每一行的buffer大小
        }
    }
    else {

        for(size_t i = 0; i < vertices_.size(); ++i) {

            std::stringstream temp_string_stream;
            temp_string_stream << vertices_.at(i)(0) << " "
                               << vertices_.at(i)(1) << " "
                               << vertices_.at(i)(2) << "\n";

            list_.push_back(std::move(temp_string_stream.str()));

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

        std::stringstream temp_string_stream;
        temp_string_stream << "3 " << vertex_indices_.at(i)(0) << " "
                                   << vertex_indices_.at(i)(1) << " "
                                   << vertex_indices_.at(i)(2) << "\n";
        list_.push_back(std::move(temp_string_stream.str()));

        string_buffer_size += sizeof(list_.at(i));      // 粗略计算每一行的buffer大小
    }
    string_buffer_.reserve(string_buffer_size);
    for(auto &b : list_) {

        string_buffer_ += b;
    }

    return string_buffer_;
}

std::vector<std::string> MeshToString::splitSubstringBySpace(std::string in_string)
{
    std::vector<std::string> sub_string;
    size_t pos {0};
    std::string space_delimiter = " ";
    while((pos = in_string.find(space_delimiter)) != std::string::npos) {

        auto word = in_string.substr(0, pos);
        sub_string.push_back(word);
        in_string.erase(0, pos + space_delimiter.length());
        // LOG(INFO) << "Getting a word = " << word;
    }
    // LOG(INFO) << "Getting a word = " << in_string;
    sub_string.push_back(in_string);   // Need to get last sub_string

    return sub_string;
}

bool MeshToString::getVertexAndTriangleSize(const std::vector<std::string> &lines, size_t &end_header_line)
{
    // Remember to remove apostrophe '
    auto ply_format = lines.at(0);
    ply_format.erase(ply_format.begin());
    header_.file_format_ = ply_format;
    header_.ply_format_ = lines.at(1);
    header_.comment_ = lines.at(2);

    // LOG(INFO) << "header:\n"
              // << "file_format_ = " << header_.file_format_ << "\n"
              // << "ply_format_  = " << header_.ply_format_ << "\n"
              // ;

    static size_t line_counter = 0;     // find line of end_header

    bool found_end_header { false };
    bool found_vertex_size { false };
    bool found_triangle_size { false };
    for(size_t i = 0; i < lines.size(); ++i) {

        // LOG(INFO) << "Processing line: " << lines.at(i);
        // Step 1, get vertex size
        if(!found_vertex_size) {

            auto l_copy = lines.at(i);
            std::vector<std::string> words {};
            words = std::move(splitSubstringBySpace(l_copy));

            if(words.size() < 3) {

                // LOG(INFO) << "Not the content wanted.";
                line_counter++;
                continue;
            }
            std::string element_vertex{ "element vertex" };
            auto compare_string = std::move(words.at(0) + " " + words.at(1));
            bool is_success = element_vertex.compare(std::move(words.at(0)));
            // LOG(INFO) << "is_success = " << is_success << ", compare_string = " << compare_string;
            if(element_vertex.compare(compare_string) == 0) {

                vertex_size_ = std::stoi(words.at(2));
                found_vertex_size = true;
                // LOG(INFO) << "Found vertex size = " << vertex_size_;
            } else {

                line_counter++;
                continue;
            }
        }
        // Step 2, get triangle size
        else if(!found_triangle_size){

            auto l_copy = lines.at(i);
            std::vector<std::string> words {};
            words = std::move(splitSubstringBySpace(l_copy));

            if(words.size() < 3) {

                // LOG(INFO) << "Not the content wanted.";
                line_counter++;
                continue;
            }
            std::string element_face{ "element face" };
            auto compare_string = std::move(words.at(0) + " " + words.at(1));
            // bool is_success = element_face.compare(compare_string);
            // LOG(INFO) << "is_success = " << is_success << ", compare_string = " << compare_string;

            if(element_face.compare(compare_string) == 0) {

                triangle_size_ = std::stoi(words.at(2));
                found_triangle_size = true;
                // LOG(INFO) << "Found triangle size = " << triangle_size_;
            }
            else {

                line_counter++;
                continue;
            }
        }

        if(header_.end_header_.compare(lines.at(i)) != 0) {

            // LOG(INFO) << "Line " << line_counter++ << " is not end_header.";
        }
        else {

            // LOG(INFO) << "end_header is found.";
            found_end_header = true;
            end_header_line = i;
            break;
        }
    }

    if(found_end_header) {

        return true;
    }
    else
        return false;
}

TriangleMeshPtr MeshToString::generateMeshFromString(const std::string &mesh_string)
{
    std::stringstream ss(mesh_string);
    auto mesh = std::make_shared<open3d::geometry::TriangleMesh>();

    // TODO When there is a large number
    std::vector<std::string> lines;
    // lines.resize(1024);
    // size_t line_number{0};
    std::string temp_line{""};
    while(std::getline(ss, temp_line)) {

        lines.push_back(temp_line);
        // LOG(INFO) << "get line number = " << line_number++ << "\n string = " << temp_line;
    }
    // line_number -= 1;       // get the right end line 

    size_t end_header_line { 0 };
    getVertexAndTriangleSize(lines, end_header_line);
    // LOG(INFO) << "get end_header_line = " << end_header_line;
    // LOG(INFO) << "get end_header_line string = " << lines.at(end_header_line);

    if(vertex_size_ == 0) {

        LOG(FATAL) << "Failed to get vertex size in mesh string";
    }

    mesh->vertices_.resize(vertex_size_);
    mesh->triangles_.resize(triangle_size_);
    auto need_start_line = end_header_line+1;
    auto need_end_line = lines.size()-1;
    size_t j = 0;
    size_t k = 0;
    bool read_vertex { false };
    for(size_t i = need_start_line; i < need_end_line; i++) {

        auto substr = splitSubstringBySpace(lines.at(i));
        if(!read_vertex) {

            if(substr.size() < 3) {

                LOG(FATAL) << "Wrong vertex format!";
                continue;
            }
            if(substr.size() == 6) {

                has_vertex_color_ = true;
            }

            if(j < mesh->vertices_.size()) {

                auto x = std::stod(substr.at(0));
                auto y = std::stod(substr.at(1));
                auto z = std::stod(substr.at(2));
                auto vertex = Eigen::Vector3d(x, y, z);
                mesh->vertices_.at(j) = std::move(vertex);
                // LOG(INFO) << "vertex = " << mesh->vertices_.at(j).transpose();

                if(has_vertex_color_) {

                    auto r = std::stod(substr.at(0));
                    auto g = std::stod(substr.at(1));
                    auto b = std::stod(substr.at(2));
                    auto vertex_color = Eigen::Vector3d(r, g, b);
                    mesh->vertex_colors_.at(j) = std::move(vertex_color);
                    // LOG(INFO) << "vertex_color = " << mesh->vertex_colors_.at(j).transpose();
                }
                j++;
            }
            else {

                read_vertex = true;
            }
        }
        if(read_vertex) {       // to read the first line of triangle

            if(k < mesh->triangles_.size()) {

                if(substr.size() != 4) {

                    LOG(FATAL) << "Wrong triangles format!";
                    continue;
                }

                auto idx1 = std::stod(substr.at(1));
                auto idx2 = std::stod(substr.at(2));
                auto idx3 = std::stod(substr.at(3));
                auto triangle = Eigen::Vector3i(idx1, idx2, idx3);
                mesh->triangles_.at(k) = std::move(triangle);
                // LOG(INFO) << "triangle = " << triangle.transpose();

                k++;
            }
        }
    }

    if(mesh->vertices_.size() == 0) {

        LOG(ERROR) << "Failed to convert string to mesh!";
        return nullptr;
    }

    return mesh;
}
