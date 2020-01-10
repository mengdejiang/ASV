/*
****************************************************************************
* testTargetDetection.cc:
* unit test for Target Detection using marine radar
* This header file can be read by C++ compilers
*
* by Hu.ZH(CrossOcean.ai)
****************************************************************************
*/

#include <iostream>

#include "../include/TargetTracking.h"
#include "common/plotting/include/matplotlibcpp.h"

void generatecircle(const double center_x, const double center_y,
                    const double radius, std::vector<double>& _circle_x,
                    std::vector<double>& _circle_y) {
  const int n = 120;
  _circle_x.resize(n);
  _circle_y.resize(n);
  for (int i = 0; i < n; ++i) {
    double t = 2 * M_PI * i / n;
    _circle_x.at(i) = center_x + radius * std::cos(t);
    _circle_y.at(i) = center_y + radius * std::sin(t);
  }
}

void testClustering() {
  double p_radius = 0.7;
  size_t p_neighbors = 3;

  std::vector<double> p_data_x = {
      1.520060, 2.022029, 2.057351, 2.264624, 1.833837, 1.642185, 2.296886,
      1.651659, 2.171928, 1.911756, 2.037709, 1.973467, 2.290475, 1.561361,
      1.974000, 2.402489, 1.989742, 1.769925, 2.337243, 1.763479, 1.877453,
      1.648136, 1.983505, 1.777163, 2.373442, 2.125649, 2.011568, 2.371343,
      1.866134, 2.407069, 2.411449, 1.529495, 2.213014, 2.158573, 2.453622,
      1.706733, 1.688246, 1.648121, 2.042832, 1.637036, 2.389355, 1.772783,
      2.430609, 1.835628, 2.010313, 2.130630, 1.908663, 1.803819, 1.876055,
      1.624722, 2.479029, 1.647363, 2.093093, 1.962334, 1.716604, 1.829242,
      1.862751, 2.232196, 1.973060, 2.322847, 2.496003, 2.252416, 2.420608,
      2.240567, 1.980673, 1.598466, 2.405742, 1.785233, 2.177794, 1.845079,
      2.375952, 1.619341, 2.405635, 1.905190, 1.745843};
  std::vector<double> p_data_y = {
      0.962128, 0.808040, 1.211364, 0.691987, 0.873777, 0.759126, 1.049943,
      1.479517, 0.833745, 0.765536, 0.885850, 0.586479, 1.157447, 1.167609,
      0.643743, 2.512324, 2.578131, 2.894308, 3.089108, 3.078625, 3.473692,
      3.203372, 3.397341, 3.081633, 2.613905, 3.245356, 3.420420, 3.280969,
      2.656021, 2.510251, 4.922576, 4.624166, 4.655022, 5.438162, 5.487703,
      5.095323, 4.716194, 5.033194, 5.456132, 4.715025, 4.519055, 5.389859,
      4.585668, 4.928907, 5.031169, 6.686596, 7.473307, 7.333536, 6.635525,
      7.233937, 6.928866, 7.127566, 6.662946, 6.812723, 6.539181, 6.924507,
      7.229120, 7.306841, 6.695993, 7.056129, 9.163995, 8.745723, 9.339556,
      8.871278, 8.529820, 9.284379, 9.410697, 9.384720, 9.324454, 8.689630,
      8.684424, 8.743305, 8.782045, 9.464194, 9.369996};

  std::shared_ptr<pyclustering::dataset> p_data =
      std::make_shared<pyclustering::dataset>();
  std::shared_ptr<pyclustering::clst::dbscan_data> ptr_output_result =
      std::make_shared<pyclustering::clst::dbscan_data>();

  for (std::size_t i = 0; i != p_data_x.size(); ++i) {
    p_data->push_back({p_data_x[i], p_data_y[i]});
  }

  pyclustering::clst::dbscan solver(p_radius, p_neighbors);
  solver.process(*p_data, *ptr_output_result);
  const pyclustering::clst::cluster_sequence& actual_clusters =
      ptr_output_result->clusters();

  // plotting
  // Set the size of output image = 1200x780 pixels
  matplotlibcpp::figure_size(800, 780);

  for (std::size_t index = 0; index != actual_clusters.size(); ++index) {
    std::vector<double> cluster_x;
    std::vector<double> cluster_y;

    for (auto& value : actual_clusters[index]) {
      cluster_x.push_back(p_data_x[value]);
      cluster_y.push_back(p_data_y[value]);
    }

    // Plot line from given x and y data. Color is selected automatically.
    matplotlibcpp::plot(cluster_x, cluster_y, ".");
  }

  matplotlibcpp::title("Clustering results");
  matplotlibcpp::show();

}  // testClustering

void testClusteringAndBall() {
  using namespace ASV::perception;

  ClusteringData Clustering_Data{
      1,  // p_radius
      3   // p_minumum_neighbors
  };
  SpokeProcessdata SpokeProcess_data{
      0.1,   // sample_time
      -1.0,  // radar_x
      0.0    // radar_y
  };

  AlarmZone Alarm_Zone{
      10,        // start_range_m
      20,        // end_range_m
      0,         // center_bearing_rad
      M_PI / 2,  // width_bearing_rad
      0xac       // sensitivity_threhold
  };
  std::vector<double> p_data_x = {
      1.520060, 2.022029, 2.057351, 2.264624, 1.833837, 1.642185, 2.296886,
      1.651659, 2.171928, 1.911756, 2.037709, 1.973467, 2.290475, 1.561361,
      1.974000, 2.402489, 1.989742, 1.769925, 2.337243, 1.763479, 1.877453,
      1.648136, 1.983505, 1.777163, 2.373442, 2.125649, 2.011568, 2.371343,
      1.866134, 2.407069, 2.411449, 1.529495, 2.213014, 2.158573, 2.453622,
      1.706733, 1.688246, 1.648121, 2.042832, 1.637036, 2.389355, 1.772783,
      2.430609, 1.835628, 2.010313, 2.130630, 1.908663, 1.803819, 1.876055,
      1.624722, 2.479029, 1.647363, 2.093093, 1.962334, 1.716604, 1.829242,
      1.862751, 2.232196, 1.973060, 2.322847, 2.496003, 2.252416, 2.420608,
      2.240567, 1.980673, 1.598466, 2.405742, 1.785233, 2.177794, 1.845079,
      2.375952, 1.619341, 2.405635, 1.905190, 1.745843};
  std::vector<double> p_data_y = {
      0.962128, 0.808040, 1.211364, 0.691987, 0.873777, 0.759126, 1.049943,
      1.479517, 0.833745, 0.765536, 0.885850, 0.586479, 1.157447, 1.167609,
      0.643743, 2.512324, 2.578131, 2.894308, 3.089108, 3.078625, 3.473692,
      3.203372, 3.397341, 3.081633, 2.613905, 3.245356, 3.420420, 3.280969,
      2.656021, 2.510251, 4.922576, 4.624166, 4.655022, 5.438162, 5.487703,
      5.095323, 4.716194, 5.033194, 5.456132, 4.715025, 4.519055, 5.389859,
      4.585668, 4.928907, 5.031169, 6.686596, 7.473307, 7.333536, 6.635525,
      7.233937, 6.928866, 7.127566, 6.662946, 6.812723, 6.539181, 6.924507,
      7.229120, 7.306841, 6.695993, 7.056129, 9.163995, 8.745723, 9.339556,
      8.871278, 8.529820, 9.284379, 9.410697, 9.384720, 9.324454, 8.689630,
      8.684424, 8.743305, 8.782045, 9.464194, 9.369996};

  TargetTracking<> Target_Tracking(Alarm_Zone, SpokeProcess_data,
                                   Clustering_Data);

  auto TargetDetection_RTdata =
      Target_Tracking.TestClustering(p_data_x, p_data_y)
          .getTargetDetectionRTdata();

  // plotting
  // Set the size of output image = 1200x780 pixels
  matplotlibcpp::figure_size(800, 780);

  for (std::size_t index = 0; index != TargetDetection_RTdata.target_x.size();
       ++index) {
    std::vector<double> circle_x;
    std::vector<double> circle_y;

    generatecircle(
        TargetDetection_RTdata.target_x[index],
        TargetDetection_RTdata.target_y[index],
        std::sqrt(TargetDetection_RTdata.target_square_radius[index]), circle_x,
        circle_y);
    // Plot line from given x and y data. Color is selected automatically.
    matplotlibcpp::plot(circle_x, circle_y, "-");
  }
  matplotlibcpp::plot(p_data_x, p_data_y, ".");

  matplotlibcpp::title("Clustering and MiniBall results");
  matplotlibcpp::axis("equal");
  matplotlibcpp::show();

}  // testClusteringAndBall

void testSpokeAndCluster() {
  using namespace ASV::perception;

  SpokeProcessdata SpokeProcess_data{
      0.1,   // sample_time
      -1.0,  // radar_x
      0.0    // radar_y
  };

  AlarmZone Alarm_Zone{
      10,            // start_range_m
      20,            // end_range_m
      0,             // center_bearing_rad
      2 * M_PI / 3,  // width_bearing_rad
      0xe0           // sensitivity_threhold
  };

  ClusteringData Clustering_Data{
      1,  // p_radius
      2   // p_minumum_neighbors
  };

  SpokeProcessRTdata SpokeProcess_RTdata;
  TargetDetectionRTdata TargetDetection_RTdata;

  constexpr std::size_t size_array = 512;

  TargetTracking<> Target_Tracking(Alarm_Zone, SpokeProcess_data,
                                   Clustering_Data);

  for (int i = 0; i != 3; ++i) {
    uint8_t _value = 252 - 15 * i;
    uint8_t spokedata[size_array] = {
        0xc0,   0x00,   _value, _value, _value, _value, _value,
        _value, _value, _value, _value, _value, _value, _value,
        _value, _value, _value, _value, _value, _value, _value,
        _value, _value, _value, 0xca,   0x01,   0x30,   0x45};
    double spokeazimuth_deg = -30 + 5 * i;
    double spoke_samplerange_m = 0.5;
    double vessel_x_m = 0;
    double vessel_y_m = 0;
    double vessel_theta_rad = 0;
    Target_Tracking.AutoTracking(spokedata, size_array, spokeazimuth_deg,
                                 spoke_samplerange_m, vessel_x_m, vessel_y_m,
                                 vessel_theta_rad);

    if (i == 18) SpokeProcess_RTdata = Target_Tracking.getSpokeProcessRTdata();
    // std::size_t num = SpokeProcess_RTdata.surroundings_bearing_rad.size();
    // assert(num == SpokeProcess_RTdata.surroundings_range_m.size());
    // assert(num == SpokeProcess_RTdata.surroundings_x_m.size());
    // assert(num == SpokeProcess_RTdata.surroundings_y_m.size());
    // for (std::size_t i = 0; i != num; ++i) {
    //   std::cout << "bearing_rad: "
    //             << SpokeProcess_RTdata.surroundings_bearing_rad[i] <<
    //             std::endl;
    //   std::cout << "range_m: " << SpokeProcess_RTdata.surroundings_range_m[i]
    //             << std::endl;
    //   std::cout << "x: " << SpokeProcess_RTdata.surroundings_x_m[i]
    //             << std::endl;
    //   std::cout << "y: " << SpokeProcess_RTdata.surroundings_y_m[i]
    //             << std::endl;
    // }
    TargetDetection_RTdata = Target_Tracking.getTargetDetectionRTdata();
    std::cout << TargetDetection_RTdata.target_x.size() << std::endl;
  }

  for (int i = 3; i != 20; ++i) {
    uint8_t _value = 100 * i - 60;
    uint8_t spokedata[size_array] = {
        0xc0,   0x00,   _value, _value, _value, _value, _value,
        _value, _value, _value, _value, _value, _value, _value,
        _value, _value, _value, _value, _value, _value, _value,
        _value, _value, _value, 0xca,   0x01,   0x30,   0x45};
    double spokeazimuth_deg = -30 + 5 * i;
    double spoke_samplerange_m = 0.5;
    double vessel_x_m = 0;
    double vessel_y_m = 0;
    double vessel_theta_rad = 0;
    Target_Tracking.AutoTracking(spokedata, size_array, spokeazimuth_deg,
                                 spoke_samplerange_m, vessel_x_m, vessel_y_m,
                                 vessel_theta_rad);

    if (i == 18) SpokeProcess_RTdata = Target_Tracking.getSpokeProcessRTdata();
    // std::size_t num = SpokeProcess_RTdata.surroundings_bearing_rad.size();
    // assert(num == SpokeProcess_RTdata.surroundings_range_m.size());
    // assert(num == SpokeProcess_RTdata.surroundings_x_m.size());
    // assert(num == SpokeProcess_RTdata.surroundings_y_m.size());
    // for (std::size_t i = 0; i != num; ++i) {
    //   std::cout << "bearing_rad: "
    //             << SpokeProcess_RTdata.surroundings_bearing_rad[i] <<
    //             std::endl;
    //   std::cout << "range_m: " << SpokeProcess_RTdata.surroundings_range_m[i]
    //             << std::endl;
    //   std::cout << "x: " << SpokeProcess_RTdata.surroundings_x_m[i]
    //             << std::endl;
    //   std::cout << "y: " << SpokeProcess_RTdata.surroundings_y_m[i]
    //             << std::endl;
    // }
    TargetDetection_RTdata = Target_Tracking.getTargetDetectionRTdata();

    std::cout << TargetDetection_RTdata.target_x.size() << std::endl;
  }

  // for (int i = 0; i != 20; ++i) {
  //   double spokeazimuth_deg = -30 + 5 * i;
  //   uint8_t _value = 240 + 5 * i;
  //   std::cout << "angle: " << spokeazimuth_deg << std::endl;
  //   printf("%x\n", _value);
  // }

  // plotting
  // Set the size of output image = 1200x780 pixels
  matplotlibcpp::figure_size(800, 780);

  for (std::size_t index = 0; index != TargetDetection_RTdata.target_x.size();
       ++index) {
    std::vector<double> circle_x;
    std::vector<double> circle_y;

    generatecircle(
        TargetDetection_RTdata.target_x[index],
        TargetDetection_RTdata.target_y[index],
        std::sqrt(TargetDetection_RTdata.target_square_radius[index]), circle_x,
        circle_y);
    // Plot line from given x and y data. Color is selected automatically.
    matplotlibcpp::plot(circle_y, circle_x, "-");
  }
  matplotlibcpp::plot(SpokeProcess_RTdata.surroundings_y_m,
                      SpokeProcess_RTdata.surroundings_x_m, ".");

  matplotlibcpp::title("Clustering and MiniBall results");
  matplotlibcpp::axis("equal");
  matplotlibcpp::show();

}  // testSpokeAndCluster

int main() {
  // testSpokeAndCluster();
  testClusteringAndBall();
}