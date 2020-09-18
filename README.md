# imagePipeline3
Image Pipeline

To run:
```
make && ./pipeline.out -c N_IMAGES -u BINARY_THRESHOLD -n RATE_THRESHOLD -m TEXT_FILE_NAME -b
```
Where:
* N_IMAGES: Integer with the number of images to use with the pipeline (1...*)
* BINARY_THRESHOLD: Integer with the Threshold to convert to binary (0...255)
* RATE_THRESHOLD: Integer with the Threshold to rate the image (percentage) (0...100)
* TEXT_FILE_NAME: The name of the file with the laplace matrix i.e:
```
0 1 0 
2 -10 2
0 1 2
```

All the images has to be in the project root, with naming like "Imagen_1.jpg","Imagen_2.jpg","Imagen_3.jpg",etc.

This project uses the library [STB](https://github.com/nothings/stb). And the makefile created by [Nicolas Gutierrez](https://github.com/ngutierrezp/Makefile).


Project Page on Github [ealopezg/imagePipeline](https://github.com/ealopezg/imagePipeline)

