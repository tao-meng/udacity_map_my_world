#include "ros/ros.h"
#include "ball_chaser/DriveToTarget.h"
#include <sensor_msgs/Image.h>

// Define a global client that can request services
ros::ServiceClient client;

// This function calls the command_robot service to drive the robot in the specified direction
void drive_robot(float lin_x, float ang_z)
{
    // TODO: Request a service and pass the velocities to it to drive the robot
	//ROS_INFO_STREAM("Drive the bot");
	
	ball_chaser::DriveToTarget srv;
	srv.request.linear_x = lin_x;
	srv.request.angular_z = ang_z;
	
	 // Call the drive_bot service and pass the requested joint angles
    if (!client.call(srv))
        ROS_ERROR("Failed to call service drive_bot");
}

// This callback function continuously executes and reads the image data
void process_image_callback(const sensor_msgs::Image img)
{

    int white_pixel = 255;

    // TODO: Loop through each pixel in the image and check if there's a bright white one
    // Then, identify if this pixel falls in the left, mid, or right side of the image
    // Depending on the white ball position, call the drive_bot function and pass velocities to it
    // Request a stop when there's no white ball seen by the camera
	
	// Answer:
	bool found_white = false;
	// Define a tolerance threshold to turn left or right
	double tolerance = 0.05;
	
	// bytes of each pixel, not sure it's RGB or RGBA
	int depth = img.step / img.width;
	/*
	// Loop through each pixel in the image and check if there's a bright white one
    for (int i = 0; i < img.height * img.width; i++) {
		// RGBA or RGB
		found_white = true;
		for (int j = 0; j < depth; j++) {
			if (img.data[i * depth + j] != white_pixel) {
				found_white = false;
				break;
			}
		}
		
		if (found_white == true) {
			int delt = (img.width / 2) - (i % img.width);
			float x = (float)delt / img.width / 2;
			//ROS_INFO("img width %d, height %d, delt %d, x %f", img.width / 2, i % img.width, delt, x);
			
			// in middle of the image, go
			if (fabs(x) < tolerance) {
				ROS_INFO("Go forward");
				drive_robot(0.5, 0.0);
			}
			else	// turn left or right
			{
				
				x = x / fabs(x) * 0.1;
				ROS_INFO("Turn left or right - angular:%1.2f", x);
				//drive_robot(0.5, x);
				drive_robot(0.0, x);
			}
			
			//drive_robot(0.5, x);
			
			break;
		}
		
		found_white = false;
    }
	*/
	// After the codes were reviewed by Teacher in Udacity
	
	// set the default left and right boundary of the white areas to middle of the image
	int left = img.width;
	int right = -1;
	int delt, j;
	
	// Loop through each pixel in the image and check if there's a bright white one
    for (int i = 0; i < img.height * img.width; i++) {
		
		// I saw the comments from Teacher,
		// but I think here is correct..., I indeed checked all RGB or RGBA of a pixel, not only the R value
		// here the logic is:
		// in RGB or RGBA (use: depth to loop), if any one is not 255, found_white will not be set to true.
		
		// RGBA or RGB
		for (j = 0; j < depth; j++) {
			if (img.data[i * depth + j] != white_pixel) {
				break;
			}
		}
		
		// all RGB or RGBA of a pixel are 255
		if (j == depth) {
			
			found_white = true;
			
			// get the current column position;
			delt = i % img.width;
			// update the left or right boundary value
			if (delt < left) {
				left = delt;
			}
			
			if (delt > right) {
				right = delt;
			}
		}
    }
	
	// take action
	if (found_white == false) {
		ROS_INFO("Stop the robot");
		drive_robot(0, 0);
	}
	else
	{
		// the middle column of the white area: (left + right) / 2
		// check the delt distance
		delt = (img.width / 2) - ((right + left) / 2);
		float x = (float)delt / img.width / 2;
		//ROS_INFO("img width %d, height %d, delt %d, x %f", img.width / 2, i % img.width, delt, x);
		
		// in middle of the image, go
		if (fabs(x) < tolerance) {
			ROS_INFO("Go forward");
			drive_robot(0.5, 0.0);
		}
		else	// turn left or right
		{
			
			x = x / fabs(x) * 0.1;
			ROS_INFO("Turn left or right - angular:%1.2f", x);
			drive_robot(0.0, x);
		}
	}
}

int main(int argc, char** argv)
{
    // Initialize the process_image node and create a handle to it
    ros::init(argc, argv, "process_image");
    ros::NodeHandle n;

    // Define a client service capable of requesting services from command_robot
    client = n.serviceClient<ball_chaser::DriveToTarget>("/ball_chaser/command_robot");

    // Subscribe to /camera/rgb/image_raw topic to read the image data inside the process_image_callback function
    ros::Subscriber sub1 = n.subscribe("/camera/rgb/image_raw", 10, process_image_callback);

    // Handle ROS communication events
    ros::spin();

    return 0;
}