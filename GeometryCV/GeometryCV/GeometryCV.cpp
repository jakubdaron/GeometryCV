#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <iomanip>
#include <cmath>

cv::Mat preProcessing(cv::Mat img)
{
    cv::Mat imgPre;
    // Zamiana na kolorów obrazu na skale szaroœci w celu umo¿liwnienia detekcji Canny
    cv::cvtColor(img, img, cv::COLOR_BGR2GRAY);

    // Zastosowanie rozmazywania gaussowskiego w celu wyg³adzenia krawedzi monet
    cv::GaussianBlur(img, imgPre, cv::Size(5, 5), 3);

    // Detekcja krawêdzi o okreœlonej czu³oœci, dostosowany na podstawie eksperymentów
    cv::Canny(imgPre, imgPre, 170, 170);

    // Rozszerzanie (dylatacja) konturów na obrazie, aby po³¹czyæ s¹siednie piksele i wype³niæ luki.
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
    cv::dilate(imgPre, imgPre, kernel, cv::Point(-1, -1), 1);

    //Operacja morfologiczna zamkniêcia, zastosowana w celu zamkniêcia ma³ych luk i usuwania ma³ych obiektów na obrazie.
    cv::morphologyEx(imgPre, imgPre, cv::MORPH_CLOSE, kernel);

    return imgPre;
}

std::string rozpoznajFigury(const std::vector<cv::Point>& contour)
{
    int numVertices = contour.size();

    if (numVertices == 4)
    {
        double side1 = cv::norm(contour[0] - contour[1]);
        double side2 = cv::norm(contour[1] - contour[2]);
        double side3 = cv::norm(contour[2] - contour[3]);
        double side4 = cv::norm(contour[3] - contour[0]);

        double maxSide = std::max({ side1, side2, side3, side4 });
        double minSide = std::min({ side1, side2, side3, side4 });

        //sprawdz roznice najkrotszego i najdluzszego boku czworoboku, aby wyrózniæ który jest kwadratem 
        if (std::abs(maxSide - minSide) < 20)
        {
            return "Kwadrat";
        }
        else
        {
            return "Prostokat";
        }
    }
    else if (numVertices == 3)
    {
        return "Trojkat";
    }
    else if (numVertices > 4)
    {
        return "Okrag";
    }
}


std::vector<cv::Point> aproksymacjaKonturu(const std::vector<cv::Point>& contour)
{
    std::vector<cv::Point> approx;
    cv::approxPolyDP(contour, approx, cv::arcLength(contour, true) * 0.03, true);
    return approx;
}

void okreslGeometrie(cv::Mat image, const std::vector<std::vector<cv::Point>>& contours, double area, size_t closestContourIndex)
{
    double skala = 5 / std::sqrt(area);

    for (size_t j = 0; j < contours.size(); ++j)
    {
        double areaSmall = cv::contourArea(contours[j]);

        // Zignoruj ma³e kontury
        if (areaSmall < 500)
            continue;

        // SprawdŸ, czy kontur jest wystarczaj¹co du¿y w porównaniu do konturu referencyjnego
        double areaRatio = areaSmall / area;
        if (areaRatio < 0.1)
            continue;

        if (j == closestContourIndex)
            break;
        else
        {
            std::string figura = rozpoznajFigury(contours[j]);
            cv::putText(image, figura, contours[j][0], cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0, 0, 255), 2); //wyswietlanie rozpoznanych figur geometrycznych

            if (figura == "Trojkat" || figura == "Prostokat" || figura == "Kwadrat")
            {
                for (size_t i = 0; i < contours[j].size(); ++i)
                {
                    cv::circle(image, contours[j][i], 3, cv::Scalar(255, 0, 0), -1);

                    cv::Point pt1 = contours[j][i];
                    cv::Point pt2 = contours[j][(i + 1) % contours[j].size()];
                    double distance = cv::norm(pt1 - pt2);

                    distance = distance * skala; //przeskalowanie d³ugoœci boku wzglêdem referencji

                    cv::Point textPos = (pt1 + pt2) / 2;
                    cv::Point textDir = pt2 - pt1;
                    cv::Point textOffset = cv::Point(-10, -10);

                    std::stringstream ss;
                    ss << std::fixed << std::setprecision(1) << distance;
                    std::string distanceString = ss.str();

                    cv::putText(image, distanceString + " mm", textPos + textOffset, cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 255), 2); //wyswietlenie dlugosci bokow w mm
                }
            }
            else if (figura == "Okrag")
            {
                cv::Point2f center;
                float radius;
                cv::minEnclosingCircle(contours[j], center, radius);

                radius = radius * skala; //przeskalowanie dlugosci promienia wzgledem referencji

                std::stringstream ss;
                ss << std::fixed << std::setprecision(1) << radius;
                std::string radiusString = ss.str();

                cv::putText(image, "R=" + radiusString + " mm", center, cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 255), 2); //wyswietlenie promienia w mm
            }
        }
    }
}

int main()
{
    std::vector<std::string> imagePaths = { "CV\\k1.jpg",
                                            "CV\\k2.jpg",
                                            "CV\\k3.jpg" };

    int currentImageIndex = 0;
    int numImages = imagePaths.size();

    cv::Mat image = cv::imread(imagePaths[currentImageIndex]);

    if (image.empty())
    {
        std::cout << "Nie mo¿na wczytaæ obrazu." << std::endl;
        return -1;
    }

    cv::Mat imgPre = preProcessing(image);

    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(imgPre, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    // Utworzenie wektora do przechowywania uproszczonych konturów
    std::vector<std::vector<cv::Point>> simplifiedContours;

    //uproszczenie konturów 
    for (const auto& contour : contours)
    {
        std::vector<cv::Point> simplifiedContour = aproksymacjaKonturu(contour);
        simplifiedContours.push_back(simplifiedContour);
    }

    double minDistance = std::numeric_limits<double>::max();
    size_t closestContourIndex = 0;

    //szukanie konturu referencyjnego najbli¿ej prawego górnego rogu
    for (size_t i = 0; i < contours.size(); ++i)
    {
        const auto& contour = contours[i];
        for (const auto& point : contour)
        {
            double distance = cv::norm(point - cv::Point(image.cols, 0)); // Odleg³oœæ od prawego górnego rogu

            if (distance < minDistance)
            {
                minDistance = distance;
                closestContourIndex = i;
            }
        }
    }

    double area = cv::contourArea(contours[closestContourIndex]);

    while (true)
    {
        cv::Mat imageToShow = image.clone();
        cv::Mat imgPreToShow = imgPre.clone();

        // Wyœwietlanie konturów na obrazie
        for (size_t i = 0; i < contours.size(); ++i)
        {
            if (i == closestContourIndex) //wyj¹tek dla konturu referencyjnego
            {
                cv::drawContours(imageToShow, contours, static_cast<int>(closestContourIndex), cv::Scalar(0, 255, 255), 3);
                cv::putText(imageToShow, "Referencja", contours[closestContourIndex][0] + cv::Point(-140, 0), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0, 255, 255), 2);
            }
            else
            {
                cv::drawContours(imageToShow, std::vector<std::vector<cv::Point>>{contours[i]}, -1, cv::Scalar(0, 0, 255), 3);
            }
        }

        okreslGeometrie(imageToShow, simplifiedContours, area, closestContourIndex);

        cv::namedWindow("Oryginalny obraz", cv::WINDOW_NORMAL);
        cv::imshow("Oryginalny obraz", imageToShow);

        cv::namedWindow("Obraz po preprocessingu", cv::WINDOW_NORMAL);
        cv::imshow("Obraz po preprocessingu", imgPreToShow);

        int key = cv::waitKey(0);

        if (key == 27) // Esc - Wyjœcie z programu
            break;
        else if (key == 32) // Spacja - Prze³¹czanie obrazków
        {
            currentImageIndex = (currentImageIndex + 1) % numImages;
            image = cv::imread(imagePaths[currentImageIndex]);

            if (image.empty())
            {
                std::cout << "Nie mo¿na wczytaæ obrazu." << std::endl;
                break;
            }

            imgPre = preProcessing(image);

            contours.clear();
            cv::findContours(imgPre, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

            simplifiedContours.clear();
            for (const auto& contour : contours)
            {
                std::vector<cv::Point> simplifiedContour = aproksymacjaKonturu(contour);
                simplifiedContours.push_back(simplifiedContour);
            }

            minDistance = std::numeric_limits<double>::max();
            closestContourIndex = 0;

            for (size_t i = 0; i < contours.size(); ++i)
            {
                const auto& contour = contours[i];
                for (const auto& point : contour)
                {
                    double distance = cv::norm(point - cv::Point(image.cols, 0)); // Odleg³oœæ od prawego górnego rogu

                    if (distance < minDistance)
                    {
                        minDistance = distance;
                        closestContourIndex = i;
                    }
                }
            }

            area = cv::contourArea(contours[closestContourIndex]);
        }
    }

    cv::destroyAllWindows();
    return 0;
}
