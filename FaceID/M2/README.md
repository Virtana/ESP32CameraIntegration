## How to Train the model using GOOGLE COLAB

1. Add Training & Testing dataset to GDrive
    * In the train folder, every different class should have its own subfolder of images
    * An example of the folder’s subdirectory is shown below

2. Run cells until the Mount Google Drive subsection
    * Go to this URL specified, agree to the terms and copy the authorization code. 
    * Paste this code in the input box in the colab notebook.

3. Run the rest of cells
    * Under the  "Training the model" section you will see the epochs running when the model is being trained.
    * Under the “Check Predictions” section, you will see a visual representation of a small subset of images that the model predicted and its accuracy 

4. The model and converted TFLite model files will be exported to your Google Drive in a Models folder. You can change the export path to customize this.
