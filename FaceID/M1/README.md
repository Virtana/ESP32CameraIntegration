Milestone 1: Data Labelling

In order to train an ML model, we first need to compile a set of test data. Given a set of camera images, write a simple application to facilitate 
labelling images with a user input. This application should output a csv file in a format that is compatible for future model development.

NB: For AWS SageMaker, csv files should contain no headers and assign the target value to the first column followed by the feature columns.
    Processing should be taken into consideration to use numerical values as classes rather than string values for name labels.
    
This Vue application is designed to dynamically allow users to assign an appropriate label to training data that can be used for the model later on.

TBD: Use predefined classes to mitigate human input errors && for better usability
