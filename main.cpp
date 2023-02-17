#include "crow.h"
#include "crow/app.h"
#include <opencv2/opencv.hpp>

int main() 
{
	crow::SimpleApp app;
	cv::Mat img;

	CROW_WEBSOCKET_ROUTE(app, "/")
	.onopen([&](crow::websocket::connection& conn){
		std::cout << "New connection\n";
	})
	.onmessage([&](crow::websocket::connection& /*conn*/, const std::string& data, bool is_binary){
		std::vector<uchar> char_data(data.begin(), data.end());
		img = cv::imdecode(char_data, cv::IMREAD_UNCHANGED);
		cv::cvtColor(img, img, cv::COLOR_BGR2GRAY);

		cv::Mat blurred;
		cv::GaussianBlur(img, blurred, cv::Size(0, 0), 50, 50);

		cv::divide(img, blurred, img, 255);
		cv::threshold(img, img, 0, 255, cv::THRESH_BINARY + cv::THRESH_OTSU);

		std::vector<std::vector<cv::Point>> contours;
		cv::findContours(img, contours, 1, 2);

		std::vector<cv::Rect> rects;

		for (auto &contour : contours) 
		{
			cv::Rect r = cv::boundingRect(contour);
			if (r.width == img.cols && r.height == img.rows) continue;

			rects.push_back(r);
		}

		std::sort(rects.begin(), rects.end(), [](const cv::Rect &r1, const cv::Rect &r2){
			return r1.x < r2.x;
		});

	
		// merge overlapping rectangles
		int size = rects.size();
		for (int i = 0; i < size; i++) 
		{
			int sx0 = rects[i].x;
			int w0 = rects[i].width;

			int sx1 = rects[i + 1].x;
			int w1 = rects[i + 1].width;

			if (sx0 < sx1 && sx1 + w1 < sx0 + w0) 
			{
				rects.erase(std::next(rects.begin() + i));
				i--;
			}
		}

		// merge near rectangles
		/*
		for (int i = 0; i < rects.size(); i++) 
		{
			if (rects[1].x - (rects[0].x + rects[0].width) < 100) 
			{
				rects[0].width = (rects[1].x + rects[1].width) - rects[0].x;
				rects.erase(std::next(rects.begin()));
			}
		}	
		*/

		size = rects.size();
		for (int i = 0; i < size; i++) 
		{
			cv::putText(img, std::to_string(i), {rects[i].x, rects[i].y}, cv::FONT_HERSHEY_COMPLEX_SMALL, 1, cv::Scalar());
			cv::rectangle(img, rects[i], cv::Scalar(), 2);
			cv::line(img, {rects[i].x, rects[i].y}, {rects[i + 1].x, rects[i].y}, cv::Scalar(), 2);
			cv::putText(img, std::to_string(rects[i + 1].x - (rects[i].x + rects[i].width)), {rects[i].x + 10, rects[i].y}, cv::FONT_HERSHEY_COMPLEX_SMALL, 2, cv::Scalar());
		}

		/*
		for (auto &r : rects)
		{
			printf("%d, %d, %d, %d\n", r.x, r.y, r.width, r.height);
			cv::rectangle(img, r, cv::Scalar(), 2);
		}
		*/
		cv::imwrite("andew.jpg", img);
	});	

	app.port(3000).multithreaded().run();

	return 0;
}
