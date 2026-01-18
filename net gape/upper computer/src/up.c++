#include "net_gape.h"
#include <rclcpp/rclcpp.hpp>
#include "std_msgs/msg/string.hpp"
#include <memory>

class Communicate : public rclcpp::Node {
public:
    std::unique_ptr<cl> socket;  // 使用智能指针管理
    std::string ip;
    std::string topic;
    int port;
    int timeout;
    char recv_buffer[6];  // 接收缓冲区
    
    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr topic_sub;
    rclcpp::TimerBase::SharedPtr timer_receive;
    
    void timer_callback();
    void message_callback(const std_msgs::msg::String::SharedPtr msg);
    
    Communicate() : Node("communicate") {
        // 先获取参数
        ip = this->declare_parameter<std::string>("ip", "192.168.10.32");
        port = this->declare_parameter<int>("port", 3000);
        topic = this->declare_parameter<std::string>("topic", "def_topic");
        timeout = this->declare_parameter<int>("timeout", 5);
        
        RCLCPP_INFO(this->get_logger(), "参数配置: ip=%s, port=%d, topic=%s, timeout=%d", 
                   ip.c_str(), port, topic.c_str(), timeout);
        
        try {
            // 创建socket对象
            socket = std::make_unique<cl>(ip.c_str(), port);
            
            // 连接服务器
            RCLCPP_INFO(this->get_logger(), "正在连接...");
            if (!socket->connect_cl()) {
                RCLCPP_ERROR(this->get_logger(), "连接失败");
                rclcpp::shutdown();
                return;
            }
            RCLCPP_INFO(this->get_logger(), "连接成功");
            
        } catch (const std::exception& e) {
            RCLCPP_FATAL(this->get_logger(), "初始化失败: %s", e.what());
            rclcpp::shutdown();
            return;
        }
        
        // 创建订阅者（使用std_msgs::msg::String）
        topic_sub = this->create_subscription<std_msgs::msg::String>(
            topic,
            10,  // QoS深度，不是SensorDataQoS
            std::bind(&Communicate::message_callback, this, std::placeholders::_1)
        );
        RCLCPP_INFO(this->get_logger(), "已订阅话题: %s", topic.c_str());
        
        // 创建定时器（接收数据）
        timer_receive = this->create_wall_timer(
            std::chrono::milliseconds(100),
            std::bind(&Communicate::timer_callback, this)
        );
        RCLCPP_INFO(this->get_logger(), "定时器已启动，周期: 100ms");
    }
};

void Communicate::timer_callback() {
    if (!socket) {
        RCLCPP_WARN(this->get_logger(), "Socket未初始化");
        return;
    }
    
    bool success = socket->receiveDoublesWithTimeout(recv_buffer, timeout);
    if (success) {
        // 处理接收到的数据
        RCLCPP_INFO(this->get_logger(), "收到数据: [%c%c%c%c%c%c]", 
                   recv_buffer[0], recv_buffer[1], recv_buffer[2],
                   recv_buffer[3], recv_buffer[4], recv_buffer[5]);
        
        // 可以发布到ROS话题
        // auto msg = std_msgs::msg::String();
        // msg.data = std::string(recv_buffer, 6);
        // publisher_->publish(msg);
    }
}

void Communicate::message_callback(const std_msgs::msg::String::SharedPtr msg) {
    if (!socket) {
        RCLCPP_WARN(this->get_logger(), "Socket未初始化,无法发送");
        return;
    }
    
    // 将字符串转换为char数组（最多6个字符）
    char send_data[6] = {0};
    size_t len = std::min(msg->data.length(), size_t(6));
    std::copy_n(msg->data.begin(), len, send_data);
    
    RCLCPP_INFO(this->get_logger(), "发送数据: %s", msg->data.c_str());
    
    if (!socket->sendDoubles(send_data)) {
        RCLCPP_ERROR(this->get_logger(), "发送失败");
    }
}

int main(int argc, char** argv) {
    rclcpp::init(argc, argv);
    
    try {
        auto node = std::make_shared<Communicate>();
        rclcpp::spin(node);
    } catch (const std::exception& e) {
        std::cerr << "程序异常: " << e.what() << std::endl;
        return 1;
    }
    
    rclcpp::shutdown();
    return 0;
}