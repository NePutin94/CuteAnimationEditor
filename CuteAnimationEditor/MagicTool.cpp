#include "MagicTool.h"

std::vector<sf::FloatRect> MagicTool::makeBounds()
{
	std::vector<sf::FloatRect> b;
	std::vector<std::vector<cv::Point>> contours;
	std::vector<cv::Vec4i> hierarchy;
	findContours(transform_image, contours, hierarchy, cv::RETR_TREE, CV_CHAIN_APPROX_SIMPLE);
	sf::IntRect deltaOffset = offset;
	/*switch (mode)
	{
	case 0:

		break;
	case 1:
		deltaOffset = { 1,1 ,-2,-2 };
		break;
	default:
		break;
	}*/
	////addWeighted(image_2, 0.75, contours, 0.25, 0, drawing);
	////drawContours(drawing, contours, -1, cv::Scalar(255));
	//cv::Mat rect = cv::Mat::zeros(th2.size(), CV_8UC3);
	//vector<vector<cv::Point> > contours_poly(contours.size());
	//vector<cv::Rect> boundRect(contours.size());
	//vector<cv::Point2f>centers(contours.size());
	//vector<float>radius(contours.size());
	for (int i = 1; i >= 0; i = hierarchy[i][0])
	{
		//print it
		//cv::drawContours(img, contours, i, cv::Scalar(255));
		//out.emplace_back(cv::boundingRect(contours[i]));
		//cv::rectangle(img, cv::boundingRect(contours[i]), cv::Scalar(120, 120, 255, 255));
		//for every of its internal contours
		//for (int j = hierarchy[i][2]; j >= 0; j = hierarchy[j][0])
		//{
		//	//	//recursively print the external contours of its children
		//	printExternalContours(img, contours, hierarchy, hierarchy[j][2]);
		//}
		//vector<cv::Point> poly;
		//approxPolyDP(contours[i], poly, 5, true);
		auto r = cv::boundingRect(contours[i]);
		sf::FloatRect rect;
		rect.left = r.x + deltaOffset.left;
		rect.top = r.y + deltaOffset.top;
		rect.width = r.width + deltaOffset.width;
		rect.height = r.height + deltaOffset.height;
		//rect.x -= 10;
		//rect.y -= 10;
		//cv::rectangle(source_image, rect, cv::Scalar(10, 10, 255, 100));
		b.emplace_back(rect);
	}
	//printExternalContours(magicTool.source_image, contours, hierarchy, 0);
	return b;
}

void MagicTool::makeBounds(std::vector<sf::FloatRect>& b)
{
	//std::vector<sf::FloatRect> b;
	std::vector<std::vector<cv::Point>> contours;
	std::vector<cv::Vec4i> hierarchy;
	findContours(transform_image, contours, hierarchy, cv::RETR_TREE, CV_CHAIN_APPROX_SIMPLE);
	sf::IntRect deltaOffset = offset;
	/*switch (mode)
	{
	case 0:

		break;
	case 1:
		deltaOffset = { 1,1 ,-2,-2 };
		break;
	default:
		break;
	}*/
	////addWeighted(image_2, 0.75, contours, 0.25, 0, drawing);
	////drawContours(drawing, contours, -1, cv::Scalar(255));
	//cv::Mat rect = cv::Mat::zeros(th2.size(), CV_8UC3);
	//vector<vector<cv::Point> > contours_poly(contours.size());
	//vector<cv::Rect> boundRect(contours.size());
	//vector<cv::Point2f>centers(contours.size());
	//vector<float>radius(contours.size());
	for (int i = 1; i >= 0; i = hierarchy[i][0])
	{
		//print it
		//cv::drawContours(img, contours, i, cv::Scalar(255));
		//out.emplace_back(cv::boundingRect(contours[i]));
		//cv::rectangle(img, cv::boundingRect(contours[i]), cv::Scalar(120, 120, 255, 255));
		//for every of its internal contours
		//for (int j = hierarchy[i][2]; j >= 0; j = hierarchy[j][0])
		//{
		//	//	//recursively print the external contours of its children
		//	printExternalContours(img, contours, hierarchy, hierarchy[j][2]);
		//}
		//vector<cv::Point> poly;
		//approxPolyDP(contours[i], poly, 5, true);
		auto r = cv::boundingRect(contours[i]);
		sf::FloatRect rect;
		rect.left = r.x + deltaOffset.left;
		rect.top = r.y + deltaOffset.top;
		rect.width = r.width + deltaOffset.width;
		rect.height = r.height + deltaOffset.height;
		//rect.x -= 10;
		//rect.y -= 10;
		//cv::rectangle(source_image, rect, cv::Scalar(10, 10, 255, 100));
		b.emplace_back(rect);
	}
	//printExternalContours(magicTool.source_image, contours, hierarchy, 0);
}

void MagicTool::makeTransformImage()
{
	transform_image = cv::Mat::zeros(source_image.size(), source_image.type());
	for (int i = 0; i < transform_image.rows; ++i)
	{
		for (int j = 0; j < transform_image.cols; ++j)
		{
			auto p = source_image.at<cv::Vec4b>(i, j);
			if (p[3] != 0)
			{
				if (makeAllBlack)
				{
					p[0] = 0;
					p[1] = 0;
					p[2] = 0;
				}
				else
				{
					p[0] += add;
					p[1] += add;
					p[2] += add;
				}
			}
			transform_image.at<cv::Vec4b>(i, j) = p;
		}
	}

	if (gray)
		cv::cvtColor(transform_image, transform_image, CV_RGB2GRAY);

	switch (mode)
	{
	case 0:
	{
		offset = { 1,1 ,-1,-1 };
	}
	break;
	case 1:
	{
		offset = { 1,1 ,-1,-1 };
		cv::Mat sub_mat = cv::Mat::zeros(transform_image.size(), CV_8UC3);
		cv::Canny(transform_image, sub_mat, thresh, thresh * 2, 3);
		transform_image = sub_mat;
	}
	break;
	case 2:
		cv::threshold(transform_image, transform_image, thresh, 255, 0);
		break;
	case 3:
		break;
	}
	if (useMorph)
	{
		auto rect_kernel = getStructuringElement(cv::MORPH_RECT, cv::Size(kernel_rect.x, kernel_rect.y));
		cv::Mat sub_mat = cv::Mat::zeros(transform_image.size(), CV_8UC3);
		cv::morphologyEx(transform_image, sub_mat, cv::MORPH_CLOSE, rect_kernel, cv::Point(-1, -1), morph_iteration);
		transform_image = sub_mat;
	}
	cv::Mat c;
	cv::cvtColor(transform_image, c, CV_BGR2RGBA, 4);
	img.create(c.cols, c.rows, c.ptr());
}

cv::Mat MagicTool::sfml2opencv(const sf::Image& img, bool toBGRA, bool fixZeroTransp)
{
	cv::Size size(img.getSize().x, img.getSize().y);
	cv::Mat mat(size, CV_8UC4, (void*)img.getPixelsPtr(), cv::Mat::AUTO_STEP);
	if (fixZeroTransp)
		for (int i = 0; i < mat.rows; ++i)
		{
			for (int j = 0; j < mat.cols; ++j)
			{
				auto& p = mat.at<cv::Vec4b>(i, j);
				if (p[0] == 0 && p[1] == 0 && p[2] == 0 && p[3] == 0)
				{
					p[0] = 255;
					p[1] = 255;
					p[2] = 255;
					p[3] = 0;
				}
			}
		}
	if (toBGRA)
		cv::cvtColor(mat, mat, cv::COLOR_RGBA2BGRA);
	return mat.clone();
}
