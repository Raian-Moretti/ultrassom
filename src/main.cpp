#include <chrono>
#include <iostream>
#include <fstream>
#include <string>
#include <Eigen/Dense>
#include <Magick++.h>

#include "Parser/CSVFileParser.hpp"
#include "Algorithm/CGNESolver.hpp"
#include "Algorithm/CGNRSolver.hpp"

void test() {
  Eigen::MatrixXd a = CSVFileToMatrixParser("data/a.csv").parse();
  Eigen::MatrixXd M = CSVFileToMatrixParser("data/M.csv").parse();
  Eigen::MatrixXd N = CSVFileToMatrixParser("data/N.csv").parse();

  std::cout << "a:" << std::endl << a << std::endl;
  std::cout << "M:" << std::endl << M << std::endl;
  std::cout << "N:" << std::endl << N << std::endl;

  Eigen::MatrixXd MN = M * N;
  Eigen::MatrixXd aM = a * M;
  Eigen::MatrixXd Ma = M * a.transpose();

  std::cout << "M * N:"    << std::endl << MN << std::endl;
  std::cout << "a * M:"    << std::endl << aM << std::endl;
  std::cout << "M * a(T):" << std::endl << Ma << std::endl;

  Eigen::MatrixXd MN_answer = CSVFileToMatrixParser("data/MN.csv").parse();
  Eigen::MatrixXd aM_answer = CSVFileToMatrixParser("data/aM.csv").parse();

  assert(MN.isApprox(MN_answer, 0.0001));
  assert(aM.isApprox(aM_answer, 0.0001));
}

void plot(const Eigen::VectorXd& image) {
  std::ofstream image_file("out/image.pgm");
  image_file << "P2" << '\n' << 60 << ' ' << 60 << '\n' << "255" << '\n';

  for (auto j = 0u; j < 60; j++) {
      for (auto i = 0u; i < 60; i++) {
        unsigned int pixel = (unsigned int) image(60 * i + j);
        image_file << pixel << " ";
      }
      image_file << "\n";
  }

  image_file.close();
  Magick::Image image_magick;
  image_magick.read("out/image.pgm");
  image_magick.write("out/image.png");
}

std::function<void()> time_it() {
  auto start = std::chrono::system_clock::now();
  return [start]() {
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> elasped = end - start;
    std::cout << elasped.count() << " seconds" << std::endl;
  };
}

double reduction_factor(const Eigen::MatrixXd& H) {
  return (H.transpose() * H).norm();
}

double regularization_coefficient(const Eigen::VectorXd& g, const Eigen::MatrixXd& H) {
  return (H.transpose() * g).cwiseAbs().maxCoeff();
}

int main() {
  std::cout << "Parsing g:" << std::endl;
  auto finished1 = time_it();
  Eigen::VectorXd g = CSVFileToMatrixParser("data/g-3.txt").parse();
  finished1();
  std::cout << "Done!" << std::endl;

  std::cout << "Parsing H:" << std::endl;
  auto finished2 = time_it();
  Eigen::MatrixXd H = CSVFileToMatrixParser("data/H-1.txt").parse();
  finished2();
  std::cout << "Done!" << std::endl;

  std::cout << "Computing CGNR:" << std::endl;
  auto finished5 = time_it();
  CGNRSolver solver;
  Eigen::VectorXd f = solver.solve(g, H);
  f = (f.array() - f.minCoeff()) * 255/(f.maxCoeff() - f.minCoeff());
  finished5();
  std::cout << "Done!" << std::endl;
  std::cout << f.minCoeff() << " " << f.maxCoeff() << std::endl;

  plot(f);

  std::cout << "Writing aprox g:" << std::endl;
  auto finished6 = time_it();
  auto aprox_g = H * f;
  std::ofstream aprox_g_file("out/aprox_g.txt");
  aprox_g_file << aprox_g << std::endl;
  finished6();
  aprox_g_file.close();
  std::cout << "Done!" << std::endl;

  std::cout << "Writing output:" << std::endl;
  auto finished7 = time_it();
  std::ofstream image_file("out/image.txt");
  image_file << f << std::endl;
  finished7();
  image_file.close();
  std::cout << "Done!" << std::endl;
}
