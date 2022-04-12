#!/usr/bin/env python3

import time
import pyrebase
import rospy
from geometry_msgs.msg import Twist,Vector3



config = {
  "apiKey": "ZP7Kj40OB47QGISM17xKYrsh2NJgd0i22uPSpx39",
  "authDomain": "mitappinventordiffrobot.firebaseapp.com",
  "databaseURL": "https://mitappinventordiffrobot-default-rtdb.firebaseio.com/",
  "storageBucket": "mitappinventordiffrobot.appspot.com"
}

firebase = pyrebase.initialize_app(config)
db = firebase.database()

print("Send Data to Firebase Using Raspberry Pi")
print("—————————————-")
print()


# direction = {"up":0,"down":1,"left":0,"right":1}

speedratio_x,speedratio_y = 0.1,0.1
prev_x = 0
prev_yaw = 0

def Move_forward(direct,speedratio):
  direct = dict([(a,int(b)) for a,b in direct.items()])
  speedratio = dict([(a,float(b)) for a,b in speedratio.items()])
  direct.update({"down": direct["down"]*-1})
  if(direct["up"]^abs(direct["down"]) == 1):
     vel_x = (direct["down"]|direct["up"])*speedratio["x_dir"]
     global prev_x
     prev_x = vel_x
     return vel_x
  if (direct["up"]|abs(direct["down"]) == 1):
     return prev_x
  if (direct["up"]&direct["down"] == 0):
     return 0

def Turning(direct,speedratio):
  direct = dict([(a,int(b)) for a,b in direct.items()])
  speedratio = dict([(a,float(b)) for a,b in speedratio.items()])
  direct.update({"right": direct["right"]*-1})
  if(direct["left"]^abs(direct["right"]) == 1):
     yaw = (direct["left"]|direct["right"])*speedratio["z_rot"]
     global prev_yaw
     prev_yaw = yaw
     return yaw
  if (direct["left"]|abs(direct["right"]) == 1):
     return prev_yaw
  if (direct["left"]&direct["right"] == 0):
     return 0

#def convertstrtoint(strdict):
 # if isinstance(
 #   return dict([(a,int(b)) for a,b in strdict.items()])


def Main():
    pub = rospy.Publisher('cmd_vel', Twist, queue_size=10)
    rospy.init_node('clientcontrol', anonymous=True)
    rate = rospy.Rate(10) # 10hz
    while not rospy.is_shutdown():
        direction = dict(db.child("direction").get().val())
        speedratio = dict(db.child("speedratio").get().val())
        Twist_x =  Move_forward(direction,speedratio)
        Twist_yaw =  Turning(direction,speedratio)
        Twistinstance = Twist(Vector3(Twist_x,0,0),Vector3(0,0,Twist_yaw))
      #  rospy.loginfo(f'{Twistinstance}')
        pub.publish(Twistinstance)
        rate.sleep()



if __name__ == '__main__':
    try:
        Main()
    except rospy.ROSInterruptException:
        pass