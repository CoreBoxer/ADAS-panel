#include "LaneDetection.h"

LaneDetection::LaneDetection()
{

}

LaneDetection::~LaneDetection()
{

}

Mat LaneDetection::process(Mat _image)
{
    image_src = _image;
    //*************reading image******************
    Mat image;
    image = image_src ;
    if(image.empty()){
        cout <<"reading error"<<endl;
    }

    //***************gray image*******************
    Mat image_gray;
    cvtColor(image,image_gray, CV_BGR2GRAY);

    //************gaussian smoothing**************
    Mat image_gau;
    GaussianBlur(image_gray, image_gau, Size(5,5),0,0);

    //******************canny*********************
    Mat image_canny;
    Canny(image_gau, image_canny,100, 200, 3);

    //**************interesting aera*************
    Mat dstImg;
    Mat mask = Mat::zeros(image_canny.size(), CV_8UC1);
    Point PointArray[4];
    PointArray[0] = Point(0, mask.rows);
    PointArray[1] = Point(400,330);
    PointArray[2] = Point(570,330);
    PointArray[3] = Point(mask.cols, mask.rows);
    fillConvexPoly(mask,PointArray,4,Scalar(255));
    bitwise_and(mask,image_canny,dstImg);

//************************houghline*******************
    vector<Vec4i> lines;
    int rho = 1;
    double theta = CV_PI/180;
    int threshold = 30;
    int min_line_len = 100;
    int max_line_gap = 100;
    HoughLinesP(dstImg,lines,rho,theta,threshold,min_line_len,max_line_gap);
    //cout<<lines[1]<<endl;

//***************draw line update********************************
    Mat image_draw = Mat::zeros(image_canny.size(),CV_8UC3);
    vector<int> right_x, right_y, left_x, left_y;
    double slope_right_sum;
    double b_right_sum ;
    double slope_left_sum ;
    double b_left_sum ;
    double slope_right_mean;
    double slope_left_mean;
    double b_right_mean;
    double b_left_mean;
    vector<double> slope_right, slope_left,b_right, b_left;
    for(size_t i= 0;i<lines.size();i++){
        Vec4i L;
        double slope,b;
        L = lines[i];
        slope = (L[3]-L[1])*1.0/(L[2]-L[0]);
        b = L[1]-L[0]*slope;

        if (slope >=0.2){
            slope_right.push_back(slope);
            b_right.push_back(b);
          }
        else{
            slope_left.push_back(slope);
            b_left.push_back(b);
            }
        }

    slope_right_sum = accumulate(slope_right.begin(), slope_right.end(),0.0);
    b_right_sum = accumulate(b_right.begin(), b_right.end(),0.0);
    slope_left_sum = accumulate(slope_left.begin(),slope_left.end(),0.0);
    b_left_sum = accumulate(b_left.begin(),b_left.end(),0.0);
    slope_right_mean = slope_right_sum/slope_right.size();
    slope_left_mean = slope_left_sum/slope_left.size();
    b_right_mean = b_right_sum/b_right.size();
    b_left_mean = b_left_sum/b_left.size();
//    cout <<"slope_right: "<<slope_right_sum<<endl;
    double x1r = 550;
    double x2r = 850;
    double x1l = 120;
    double x2l = 425;
    int y1r = slope_right_mean * x1r + b_right_mean;
    int y2r = slope_right_mean * x2r + b_right_mean;
    int y1l = slope_left_mean * x1l + b_left_mean;
    int y2l = slope_left_mean * x2l + b_left_mean;
    line(image_draw, Point(x1r,y1r),Point(x2r,y2r),Scalar(0,0,255),5,LINE_AA);
    line(image_draw, Point(x1l,y1l),Point(x2l,y2l),Scalar(0,0,255),5,LINE_AA);

    std::vector<cv::Point> poly_points;
    cv::Mat output;

    // Create the transparent polygon for a better visualization of the lane
    image_draw.copyTo(output);
    poly_points.push_back(Point(x1l,y1l));
    poly_points.push_back(Point(x2l,y2l));
    poly_points.push_back(Point(x1r,y1r));
    poly_points.push_back(Point(x2r,y2r));
    cv::fillConvexPoly(output, poly_points, cv::Scalar(0, 0, 255), CV_AA, 0);
    cv::addWeighted(output, 0.3, image_draw, 1.0 - 0.3, 0, image_draw);

//*************mix two image*************************
    Mat image_mix = Mat::zeros(image_canny.size(),CV_8UC3);
    addWeighted(image_draw,1,image,1,0.0,image_mix);
//**************out put****************************
    return image_mix;
}

