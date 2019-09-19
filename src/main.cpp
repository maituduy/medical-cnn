#include <iostream>
#include <armadillo>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <variant>
#include <string>
#include <time.h>
#include <json.hpp>
#include <map>

#include "nn_ops.h"
#include "activation.h"
#include "f.h"
#include "layer.h"
#include "layers/input.cpp"
#include "layers/conv2d.cpp"
#include "layers/pool2d.cpp"
#include "layers/conv2d_transpose.cpp"

#include "parser.h"

#include "model.h"

using json = nlohmann::json;
using namespace arma;
using namespace ops;
using namespace f;
using namespace cv;
using namespace layer;
using namespace parser;

// template<class T3>
// arma::Mat <T3> cvMat2armaMat(cv::Mat & cvMatIn) 
// { 
//     return arma::Mat <T3> (cvMatIn.data, cvMatIn.rows, cvMatIn.cols); 
// }

class Tictoc {
    public:
        Tictoc(){};

        template<typename T>
        void start(T f, std::string title) {
            clock_t tStart = clock();
            f();
            printf("Time taken [%s]: %.2fs\n", title.c_str(), (double)(clock() - tStart)/CLOCKS_PER_SEC);
        };
};

int main() {    
    Tictoc tictoc;
    layer::Input input(Shape(10, 16,16,3));
    layer::Conv2d c1(&input, 1, 3, Padding::SAME, 2, Activation::relu);
    layer::Conv2d c2(&c1, 4, 3, Padding::SAME, 1, Activation::relu);
    layer::Conv2dTranspose c3(&c2, 1, 5, Padding::SAME, 2, Activation::relu);
    layer::Pooling2d c4(&c3, 2);
    layer::Conv2d c5(&c4, 32, 3, Padding::VALID, 1, Activation::sigmoid);
    layer::Pooling2d c6(&c5, 2, PoolingMode::AVERAGE_TF, Padding::VALID, 2);
    layer::Conv2dTranspose c7(&c6, 1, 3, Padding::VALID, 1, Activation::sigmoid);
    layer::Conv2d c8(&c7, 32, 3, Padding::VALID, 1, Activation::sigmoid);
    
    tictoc.start([&]() {
        Model model(&c8);
        // model.summary();
        model.load_weights("/home/mxw/dev/notebook/weights.dat");
        // std::get<arma::field<arma::cube>>(model.get_layers()[1]->get_weights()[0]).print();

        
        arma::field<arma::cube> in(10);
        std::ifstream input("/home/mxw/dev/notebook/input.dat");
        json j_from_bson = json::from_bson(input);

        for (json::iterator it = j_from_bson.begin(); it != j_from_bson.end(); ++it) 
            parser::Parser::parse_arma(it, &in, Shape(10,16,16,3));
        
        auto output = model.predict(in);
        output(0).print();
        // cout << output(0)(0,0,0);
        // in.print();
        // std::get<arma::field<arma::cube>>(model.get_layers()[1]->get_weights()[0]).print();
        
    }, "test");
    


    return 0;
}
