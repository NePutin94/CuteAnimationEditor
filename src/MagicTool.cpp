#include "../include/MagicTool.h"

cv::Mat CAE::MagicTool::sfml2opencv(const sf::Texture& tex, bool toBGRA, bool fixZeroTransp)
{
    auto img = tex.copyToImage();
    cv::Size size(img.getSize().x, img.getSize().y);
    cv::Mat mat = cv::Mat(size, CV_8UC4, (void*) img.getPixelsPtr(), cv::Mat::AUTO_STEP);

    if(fixZeroTransp)
        for(int i = 0; i < mat.rows; ++i)
        {
            for(int j = 0; j < mat.cols; ++j)
            {

                if(auto& p = mat.at<cv::Vec4b>(i, j);
                        p[0] == 0 && p[1] == 0 && p[2] == 0 && p[3] == 0)
                {
                    p[0] = 255;
                    p[1] = 255;
                    p[2] = 255;
                    p[3] = 0;
                }
            }
        }
    if(toBGRA)
        cv::cvtColor(mat, mat, cv::COLOR_RGBA2BGRA);
    return mat.clone();
}

void CAE::MagicTool::makeTransformImage()
{
    transform_image = cv::Mat::zeros(use_image.size(), use_image.type());

    for(int i = 0; i < transform_image.rows; ++i)
    {
        for(int j = 0; j < transform_image.cols; ++j)
        {
            auto p = use_image.at<cv::Vec4b>(i, j);
            if(p == transp_color || p == transp_color2)
            {
//                if(makeAllBlack)
//                {
                p[0] = 255;
                p[1] = 255;
                p[2] = 255;
//                } else
//                {
//                    p[0] += add;
//                    p[1] += add;
//                    p[2] += add;
//                }
            } else
            {
                p[0] = 0;
                p[1] = 0;
                p[2] = 0;
            }
            transform_image.at<cv::Vec4b>(i, j) = p;
        }
    }

    cv::cvtColor(transform_image, transform_image, CV_BGRA2GRAY);

    offset = {1, 1, -2, -2};
//    switch(mode)
//    {
//        case 0:
//        {
//            //offset = { 1,1 ,-1,-1 };
//        }
//            break;
//        case 1:
//        {
//            //offset = { 1,1 ,-1,-1 };
//            cv::Mat sub_mat = cv::Mat::zeros(transform_image.size(), CV_8UC3);
//            cv::Canny(transform_image, sub_mat, thresh, thresh * 2, 3);
//            transform_image = sub_mat;
//        }
//            break;
//        case 2:
//            cv::threshold(transform_image, transform_image, thresh, 255, 0);
//            break;
//        case 3:
//            break;
//    }
//    if(useMorph)
//    {
//        auto rect_kernel = getStructuringElement(cv::MORPH_RECT, cv::Size(kernel_rect.x, kernel_rect.y));
//        cv::Mat sub_mat = cv::Mat::zeros(transform_image.size(), CV_8UC3);
//        cv::morphologyEx(transform_image, sub_mat, cv::MORPH_CLOSE, rect_kernel, cv::Point(-1, -1), morph_iteration);
//        transform_image = sub_mat;
//    }
    //cv::Mat transform_image_2 = cv::Mat::zeros(transform_image.size(), transform_image.type());
    //cv::blur(transform_image, transform_image_2, cv::Size(1, 1));
    //transform_image = transform_image_2;
    cv::Mat c;
    cv::cvtColor(transform_image, c, CV_BGR2RGBA, 4);
    workImage.create(transform_image.cols, transform_image.rows, c.ptr());
}


std::vector<sf::FloatRect> CAE::MagicTool::makeBounds()
{
    std::vector<sf::FloatRect> b;
    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;
    try
    {
        findContours(transform_image, contours, hierarchy, cv::RETR_TREE, CV_CHAIN_APPROX_NONE);
        sf::IntRect deltaOffset = offset;
        for(int i = 1; i >= 0 && contours.size() > 0; i = hierarchy[i][0])
        {
            auto r = cv::boundingRect(contours[i]);
            if(r.width > 5 && r.height > 5)
            {
                cv::rectangle(transform_image, r, cv::Scalar(120, 120, 255, 255));
                sf::FloatRect _rect;
                _rect.left = r.x + deltaOffset.left + cr.left;
                _rect.top = r.y + deltaOffset.top + cr.top;
                _rect.width = r.width + deltaOffset.width;
                _rect.height = r.height + deltaOffset.height;
                b.emplace_back(_rect);
            }
        }
    }
    catch (...)
    {
        Console::AppLog::addLog("Make bounds throw exception!", Console::error);
    }
    return b;
}

void CAE::MagicTool::settingWindow()
{
    ImGui::BeginChild("Magic Selection Settings");
    auto convert_to_uchar = [](float val)
    {
        return (int) ((val) * 255.0f + ((val) >= 0 ? 0.5f : -0.5f));
    };
    static ImVec4 color;
    if(ImGui::ColorEdit4("transperent color #1", (float*) &color, ImGuiColorEditFlags_AlphaBar))
    {
        transp_color[0] = convert_to_uchar(color.x);
        transp_color[1] = convert_to_uchar(color.y);
        transp_color[2] = convert_to_uchar(color.x);
        transp_color[3] = convert_to_uchar(color.w);
    }
    static ImVec4 color2;
    if(ImGui::ColorEdit4("transperent color #2", (float*) &color2, ImGuiColorEditFlags_AlphaBar))
    {
        transp_color2[0] = convert_to_uchar(color2.x);
        transp_color2[1] = convert_to_uchar(color2.y);
        transp_color2[2] = convert_to_uchar(color2.x);
        transp_color2[3] = convert_to_uchar(color2.w);
    }

    ImGui::Text("offset");
    ImGui::InputInt("left", &offset.left);
    ImGui::InputInt("top", &offset.top);
    ImGui::InputInt("rigt", &offset.width);
    ImGui::InputInt("bottom", &offset.height);
    if(!source_image.empty())
    {
        if(ImGui::Button("Processed Image"))
            makeTransformImage();
        if(ImGui::Button("Get Rects"))
        {
            makeBounds();
            cv::imshow("th3", getTransformImage());
        }
        static sf::Texture texture;
        static sf::Sprite sprite;
        texture.loadFromImage(getTransformPreview());
        sprite.setTexture(texture, true);
        ImGui::Image(sprite);
    }
    ImGui::EndChild();
}

nlohmann::json CAE::MagicTool::save2Json()
{
    using json = nlohmann::json;
    json j;

//    j["Mode"] = mode;
//    j["UseGrayScale"] = gray;
//    j["UseAllBlack"] = makeAllBlack;
//    j["UseMorph"] = useMorph;

    //auto& morphSettings = j["Morph Settings"];
//    auto& krRekt = morphSettings["Kernel Rect"];
//    krRekt["X"] = kernel_rect.x;
//    krRekt["Y"] = kernel_rect.y;
//    morphSettings["CountIteration"] = morph_iteration;
//
//    j["Thresh"] = thresh;
    //j["Add"] = add;
    return j;
}

void CAE::MagicTool::load4Json(const nlohmann::json& j)
{
//    mode = j.at("Mode");
//    gray = j.at("UseGrayScale");
//    makeAllBlack = j.at("UseAllBlack");
//    useMorph = j.at("UseMorph");
//    auto& morphSettings = j.at("Morph Settings");
//    auto& krRekt = morphSettings.at("Kernel Rect");
//    kernel_rect.x = krRekt.at("X");
//    kernel_rect.y = krRekt.at("Y");
//    morph_iteration = morphSettings.at("CountIteration");
//    thresh = j.at("Thresh");
    // add = j.at("Add");
}

void CAE::MagicTool::cropSrc(sf::IntRect crop, bool rebuildSrc)
{
    if(crop.width < 0)
    {
        crop.left += crop.width;
        crop.width = abs(crop.width);
    }
    if(crop.height < 0)
    {
        crop.top += crop.height;
        crop.height = abs(crop.height);
    }

    int leftX = std::max(crop.left, 0);
    int rightX = std::min(crop.left + crop.width, source_image.cols);
    int topY = std::max(crop.top, 0);
    int bottomY = std::min(crop.top + crop.height, source_image.rows);
    if(leftX < rightX && topY < bottomY)
    {
        cr = {leftX, topY, rightX - leftX, bottomY - topY};
        use_image = source_image(cv::Rect(cr.left, cr.top, cr.width, cr.height));
        if(rebuildSrc)
            makeTransformImage();
        return;
    } else
        use_image = source_image;
}

void CAE::MagicTool::setImage(const sf::Texture& t, sf::IntRect crop)
{
    source_image = sfml2opencv(t, true, false);
    cropSrc(crop);
}
