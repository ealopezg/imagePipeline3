# imagePipeline3
Image Pipeline with Threads using the Producer-Consumer Problem.

To run:
```
make && ./pipeline -c N_IMAGES -h N_THREADS -u BINARY_THRESHOLD -n RATE_THRESHOLD  -b BUFFER_SIZE -f
```
Where:
* N_IMAGES: Integer with the number of images to use with the pipeline (1...*)
* N_THREADS: Integer with the number of threads
* BINARY_THRESHOLD: Integer with the Threshold to convert to binary (0...255)
* RATE_THRESHOLD: Integer with the Threshold to rate the image (percentage) (0...100)
* BUFFER_SIZE: Integer with the number of lines for the buffer

All the images has to be in the project root, with naming like "Imagen_1.jpg","Imagen_2.jpg","Imagen_3.jpg",etc.


This project uses the library [STB](https://github.com/nothings/stb).

Project Page on Github [ealopezg/imagePipeline](https://github.com/ealopezg/imagePipeline3)

