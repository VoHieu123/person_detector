import time
import serial
import cv2
from PIL import Image

# define a video capture object
vid = cv2.VideoCapture(0)
SerialObj = serial.Serial(port = "COM10",
                          baudrate = 115200)

while(True):
    time.sleep(4)
    _, frame = vid.read()
    cv2.imwrite("camera.png", frame)
    # For debugging
    cv2.imshow("Image from camera", frame)

    image = Image.open('camera.png')
    image = image.resize((96, 96))
    # Convert .png to .bmp3
    image.save("image.bmp")
    # Open .bmp image
    f = open("image.bmp", "rb")
    content = f.read()

    # Length should be 54 bytes header plus
    # 3 channels * 96 bytes width * 96 bytes height
    if len(content) == (54 + (96*96*3)):
        # For debugging
        print(len(content))
        # Remove 54 bytes header of .bmp3 image format
        content = bytes(content[54:])
        # Send the image through USB port
        SerialObj.write(content)
    else:
        raise Exception("Current image is not properly processed")

    # The 'q' button is set as the quitting button
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

# After the loop release the cap object
SerialObj.close()
vid.release()
# Destroy all the windows
cv2.destroyAllWindows()
