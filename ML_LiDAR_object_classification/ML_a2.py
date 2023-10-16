from cgi import test
import csv
from tkinter import Y
from scipy import spatial as sp
import random
from math import sqrt
import math
import numpy as np
from sklearn.ensemble import RandomForestClassifier as rfc
from sklearn import svm
from sklearn.metrics import confusion_matrix
from sklearn.feature_selection import SelectKBest, f_classif
import jakteristics as jk

def split(features, ratio):
    
    # randomly add ration % of items from features list to training list and the rest (100 - ratio)% to testing set

    # number of samples to be chosen for the training set
    nr_samples = int(len(features) * ratio / 100)
    print('\nnr samples for training: ', nr_samples)
    training = random.sample(features, nr_samples)
    testing = []
    for i in features:
        if i not in training:
            testing.append(i)

    print('\nnr in testing: ', len(testing))

    return(training, testing)

def oa(list):
    # classified = clf.predict(testing_feat)
    total_items = len(list)
    # print('real: ', testing[i][0], ' predicted ', testing[i][-1])
    correct_predictions = 0

    for i in list:
        string  = str(i[0])
        index = int(string[0])
        if index - i[-1] == 0:
            # correct prediction
            correct_predictions = correct_predictions + 1

    oa = 1/total_items * correct_predictions
    return(oa)

def mean_pc_accuracy(list, list_classes):
    total_items = len(list)
    sum = 0 
    for n in list_classes:
        count_class = 0
        correct_class = 0
        for i in list:
            # the actual value
            string  = str(i[0])
            index = int(string[0])
            if index == n:
                count_class += 1
                if index - i[-1] == 0:
                    correct_class += 1
        sum = sum + (correct_class/count_class)
    mA = 1/len(list_classes) * sum
    return(mA)


def main():
    # Read in the files

    # Path to the folder where the pointcloud files are at
    folder = "C:\\Users\\Kati\\OneDrive\\Documents\\Delft\\Machine_learning\\GEO5017-A1-Clustering\\GEO5017-A1-Clustering\\data\\pointclouds\\"
    #input_file ="\\000.xyz"
    
    # create a list of numbers from 000 to 499 to iterate over the files
    input_files = [f"{i:03}" for i in range(500)]

    # initilize list that will hold the features for each point cloud.
    feat = []
    # id of the
    for i in input_files:
        input_file = folder + str(i) + ".xyz"
        
        with open(input_file) as file:
            r = csv.reader(file, delimiter=' ')
            header = next(r)
            # create a list to hold the 3d points
            list_3d_pts = []
            for line in r:
                p = list(map(float, line)) #-- convert each str to a float
                assert(len(p) == 3)
                assert(len(p) == 3)
                list_3d_pts.append(p)
    
            #box = bbox(list_3d_pts)[0] # this returns the largest and smallest x, y z, but not points
            #min_pts = bbox(list_3d_pts)[1]
            #max_pts = bbox(list_3d_pts)[2]
            convexh = sp.ConvexHull(list_3d_pts)
            conv_area = convexh.area
            conv_vol = convexh.volume

            geometry_features = jk.compute_features(list_3d_pts, search_radius=0.5, euclidean_distance=True)
            geometry_features[np.isnan(geometry_features)] = 0
            # calculate the mean for each feature instead of storing the 14 feature falues for each point in the current point cloud
            mean_features = geometry_features.mean(axis=0)
            #x_diam = box[1]-box[0]
            #y_diam = box[3]-box[2]
            #z_diam = box[5]-box[4]

            

            # add the features to the list as a separate list, together with their id from the file
            # from mean_features the following are added: Eigenvalue sum, omnivariance, eigentropy, anisotropy, planarity, linearity
            feat.append([i, conv_area, conv_vol, mean_features[0], mean_features[1], mean_features[2], mean_features[3], mean_features[4], mean_features[5]])#, x_diam, y_diam, z_diam])

    print("Number of feature sets added: ", len(feat))

    # Create a method to produce a training and testing samples 60/40
    # Define the ratio between feature and testing dataset - the number specified here gives the % of features used for training
    sets = split(feat, 60)
    testing = sets[1]

    # for training need 2 sets: one containing only the features and other containing only the class
    train_feat = []
    train_class = []

    for i in sets[0]:   
        train_feat.append(i[1:])      
        string  = str(i[0])
        index = int(string[0])
        train_class.append(index)

    nr_classes = len(set(train_class))
    print('\n there are ', nr_classes, ' classes defined in the training data: ', set(train_class))
    
    # Feature selection - provide the nunmber k for how many classes
    train_new = SelectKBest(f_classif, k = 3).fit_transform(train_feat, train_class)
    # Random forest classifier
    clf = rfc(random_state=0)
    
    clf.fit(train_new, train_class)

    testing_feat =  []
    
    for i in testing:
       testing_feat.append([i[3], i[7], i[8]])
       # comment out line above and uncomment line below to use all the features in the classifier
       #testing_feat.append(i[1:])
    
    classified = clf.predict(testing_feat)
    
    true_values = []

    for i in range(0, len(testing)):
        testing[i].append(classified[i])
        true_values.append(int(str(testing[i][0])[0]))

    #print('\npredicted: ', classified)
    #print('\n true ', true_values)


    # Method to evaluate the results
    rfc_oa = oa(testing)
    print('\nThe overall accuracy of random forest classifier: ', rfc_oa)
    rfc_ma = mean_pc_accuracy(testing, set(train_class))
    print('\nThe ma of random forest classifier: ', rfc_ma)
    rfc_confm = confusion_matrix(true_values, classified)
    print('\nRFC confusion matrix: ', rfc_confm)


    # SVM classifier
    # Add a kernel to the classifier
    svmc = svm.SVC(kernel='poly') ##Possible kernels: linear, polynomial, rbf, sigmoid: 0.275 
    svmc.fit(train_new, train_class)
    svm_classified = svmc.predict(testing_feat)
    for i in range(0, len(testing)):
        testing[i].append(svm_classified[i])
    
    # Method to evaluate the results
    svm_oa = oa(testing)
    print('\nThe overall accuracy of svm classifier: ', svm_oa)
    svm_ma = mean_pc_accuracy(testing, set(train_class))
    print('\nThe ma of svm classifier: ', svm_ma)
    svm_confm = confusion_matrix(true_values, svm_classified)
    print('\nsvm confusion matrix: ', svm_confm)
    


if __name__ == '__main__':
  main()
