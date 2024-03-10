#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

class VideoProcessor {
private:
    VideoCapture* capture;
    Mat frame; 
    Mat background;
    Mat foregroundMask;
    Ptr<BackgroundSubtractor> bgSubtractor;

public:
    VideoProcessor(const string& filename) {
        // Allocate memory for VideoCapture dynamically
        capture = new VideoCapture(filename);
        if (!capture->isOpened()) {
            cerr << "Error: Unable to open video file " << filename << endl;
            exit(EXIT_FAILURE);
        }
        // Initialize background subtractor
        bgSubtractor = createBackgroundSubtractorMOG2();
        // Read the first frame to initialize the background model
        *capture >> background;
        // Convert the background to grayscale
        cvtColor(background, background, COLOR_BGR2GRAY);
        // Update background subtractor with the initial frame
        bgSubtractor->apply(background, foregroundMask);
    }

    void processVideo() {
        while (true) {
            // Read the next frame
            *capture >> frame;
            // Check if the frame is empty or capture fails
            if (frame.empty()) {
                if (capture->isOpened()) {
                    cout << "End of video file reached." << endl;
                }
                else {
                    cout << "Error: Unable to read frame from video." << endl;
                }
                break;
            }
            // Convert the current frame to grayscale
            Mat grayFrame;
            cvtColor(frame, grayFrame, COLOR_BGR2GRAY);
            // Update the background model
            bgSubtractor->apply(grayFrame, foregroundMask);
            // Remove foreground pixels corresponding to previous contours
            foregroundMask.setTo(0, foregroundMask == 250);
            // Apply morphological opening to remove noise
            morphologyEx(foregroundMask, foregroundMask, MORPH_OPEN, getStructuringElement(MORPH_RECT, Size(2.5, 3)));
            // Find contours in the foreground mask
            vector<vector<Point>> contours;
            findContours(foregroundMask, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
            // Draw contours on the frame
            for (const auto& contour : contours) {
                drawContours(frame, vector<vector<Point>>{contour}, -1, Scalar(0, 255, 0), 2);
            }
            // Display the frame with contours
            imshow("Moving Objects", frame);
            // Wait for 30 milliseconds and check if user wants to exit
            if (waitKey(30) == 27) {
                cout << "Video playback stopped by user." << endl;
                break;
            }
        }
    }


    ~VideoProcessor() {
        // Release the allocated memory for VideoCapture
        if (capture != nullptr) {
            delete capture;
        }
        // Close all OpenCV windows
        destroyAllWindows();
    }
};

int main() {
    string filename;
    cout << "Enter the video file path: ";
    cin >> filename;

    void* baseMemoryAddress;

    // Allocate memory using malloc
    baseMemoryAddress = malloc(400 * sizeof(long long)); // Use sizeof(long long) for x64 architecture

    if (baseMemoryAddress == nullptr) {

        cout << "Error: Memory allocation failed." << endl;
        return EXIT_FAILURE;
    }
    cout << "Base memory address: " << baseMemoryAddress << std::endl;


    // Create an object of VideoProcessor class
    VideoProcessor videoProcessor(filename);

    // Call the processVideo() method to start processing the video
    videoProcessor.processVideo();

    // Free the allocated memory once the program execution is over
    free(baseMemoryAddress);

    return EXIT_SUCCESS;
    destroyAllWindows();
}
