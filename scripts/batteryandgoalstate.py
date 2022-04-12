#!/usr/bin/env python3

import time
import pyrebase
import math
import rospy
from geometry_msgs.msg import Twist,Vector3
from std_msgs.msg import Float64
from nav_msgs.msg import Odometry
import tf 



config = {
  "apiKey": "ZP7Kj40OB47QGISM17xKYrsh2NJgd0i22uPSpx39",
  "authDomain": "mitappinventordiffrobot.firebaseapp.com",
  "databaseURL": "https://mitappinventordiffrobot-default-rtdb.firebaseio.com/",
  "storageBucket": "mitappinventordiffrobot.appspot.com"
}

firebase = pyrebase.initialize_app(config)
db = firebase.database()
Pos_x=0
Pos_y=0
prev_posX = 0
prev_posY = 0
battery = 100
previous_time=time.time()
print("Send Data to Firebase Using Raspberry Pi")
print("—————————————-")
print()


def batterycallback (data):
    battery = data.data
 
def distancefinding(Pos_x,Pos_y,Place):
    xyyaw = list(map(float,Place))
    dist =math.sqrt((Pos_x-xyyaw[0])**2+(Pos_y-xyyaw[1])**2)
    return dist



def Odomcallback(odom):
    global previous_time,prev_posX,prev_posY
 #   Pos_x = odom.pose.pose.position.x
  #  Pos_y = odom.pose.pose.position.y
  #  print(odom.pose.pose.position.x)
  #  print(odom.pose.pose.position.y)
    print(f'Posx : {Pos_x} , Posy :{Pos_y}')
    Placedict = dict(db.child("Place").get().val())
    current_time = time.time()
    if(current_time - previous_time >= 15):
     # print(Placedict)
      distance = math.sqrt((Pos_x-prev_posX)**2+(Pos_y-prev_posY)**2)
      currenttimstring = time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(current_time))
      print(distance)
      if(distance >=0.05):
        print(Placedict.items())
        distancdict = {x:distancefinding(Pos_x,Pos_y,y.strip("()").split(",")) for x,y in Placedict.items()}    
        print(distancdict)
        the_nearest = min(distancdict,key=distancdict.get)

        print("We are close to  " + the_nearest) 
        db.child("locationbeen").update({currenttimstring:the_nearest}) 
        prev_posX = Pos_x
        prev_posY = Pos_y
  
      if(odom.twist.twist.linear.x <= 0.5):
    	  print("current batter",battery)
    	  battery_log = db.child("batterylog").update({currenttimstring:battery})
      previous_time = current_time
    	  

def Main():
    rospy.init_node('Iotinterface', anonymous=True)
    rospy.Subscriber("/thebatterystatus/percentage",Float64,batterycallback,queue_size=1)
    rospy.Subscriber("/odom",Odometry,Odomcallback,queue_size=1)
    listener = tf.TransformListener()
       
    rate = rospy.Rate(1)
    while not rospy.is_shutdown():
        try:
            (trans,rot) = listener.lookupTransform('/base_link', '/odom', rospy.Time(0))
        except (tf.LookupException, tf.ConnectivityException, tf.ExtrapolationException):
            continue
        global Pos_x,Pos_y
        Pos_x = trans[0]
        Pos_y = trans[1]
        print(f'x: {Pos_x}, y :{Pos_y}')
        rate.sleep()


if __name__ == '__main__':
    try:
        Main()
    except rospy.ROSInterruptException:
        pass
        
        
