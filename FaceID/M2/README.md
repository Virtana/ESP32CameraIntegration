# Training this model using GOOGLE COLAB

1. Add Training & Testing dataset to GDrive
    * Currently, the notebook accesses the datasets from your Google Drive; which must be mounted on the notebook.
    * An example of the folder’s subdirectory is shown below; whereby in the training folder, each label class has its own subdirectory of images.
    ![ERROR LOADING - Folder Structure Image](imageDocumentation/1-folder-structure.PNG/?raw=true "Folder Structure")

2. Run cells until the Mount Google Drive subsection
    * Go to this URL specified, agree to the terms and copy the authorization code. 
    * Paste this code in the input box in the colab notebook.

3. Headless model
    * This model was built using one of TensorFlow's headless models. TensorFlow has a wide range of pre-trained models; EfficientNet being the one used for this project because it was designed with size & speed in mind.
    * https://tfhub.dev/tensorflow/efficientnet/lite0/feature-vector/2

4. Training the model
    * The model’s accuracy seems to fluctuate and plateau after 8 epochs. However, a much  larger value may be used to get different results.
    ![ERROR LOADING - Training Epochs Image](imageDocumentation/2-epochs.PNG/?raw=true "Training Epochs")

    * Under the “Check Predictions” section, you will see a visual representation of a small subset of images that the model predicted and its accuracy 
    ![ERROR LOADING - Model Predictions](imageDocumentation/3-predictions.PNG/?raw=true "Model Predictions")

5. Converting to TFLite
    * The model must go through a quantization process before it is converted into a TFLite FlatBuffer format. Post-training quantization is an optimization technique that can reduce the model size while also improving CPU and hardware accelerator latency, with little degradation in model accuracy. 
    * Then the TFLiteConverter function is used to convert the regular model and the quantized models into TFLite files
    * https://www.tensorflow.org/lite/performance/post_training_quantization

6. Export
    * The model and converted TFLite model files will be exported to your Google Drive in a Models folder. You can change the export path to customize this.
    ![ERROR LOADING - Exporting Image](imageDocumentation/5-export.PNG/?raw=true "Exporting the Model files")

7. Convert to a C byte array
    * The C byte array is a read only format that can be imported into a C project to run inference locally. Use the following command to convert your tflite model.
    * `$ xxd -i converted_model.tflite > example_model_data.cc`
