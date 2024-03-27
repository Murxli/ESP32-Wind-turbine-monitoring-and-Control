import { Button } from "@/components/ui/button"
import {
  Card,
  CardContent,
  CardDescription,
  CardFooter,
  CardHeader,
  CardTitle,
} from "@/components/ui/card"
// import { Input } from "@/components/ui/input"
// import { Label } from "@/components/ui/label"
import {
  Tabs,
  TabsContent,
  TabsList,
  TabsTrigger,
} from "@/components/ui/tabs"

import Paho from 'paho-mqtt'
import { useState,useEffect } from "react"

const DashBoard = () => {
  let client;
  const [isConnected, setIsConnected] = useState(false);

  const connectMqTT = () => {
    console.log('connecting');
    client.connect({
      onSuccess: () => {
        console.log("onConnect");
        client.subscribe("World");
        client.subscribe("esp32/pub");

        client.onMessageArrived = (message) => {
          console.log("onMessageArrived:"+message.payloadString);
        }
        client.onConnectionLost = (responseObject) => {
          console.log(responseObject);
        }
        // message = new Paho.Message("Hello");
        // message.destinationName = "World";
        // client.send(message);
        console.log('connected');
      }, 
      onFailure:(error) => {console.log(error);} }); 
  }

  useEffect(()=>{
    client = new Paho.Client("broker.hivemq.com", Number(8000), "sdkfhbuvsd vsdvcsuwdfwv sdhcs 123e2edsa121245vsdverr4tw32");
    if(!isConnected){
      connectMqTT();
    }
  },[])


  return (
    <Tabs defaultValue="current" className="">
      <TabsList className="grid grid-cols-2 w-[1000px] mx-auto">
        <TabsTrigger value="current">Current</TabsTrigger>
        <TabsTrigger value="historical">Historical</TabsTrigger>
      </TabsList>
      <TabsContent value="account">
        <Card>
          <CardHeader>
            <CardTitle>Account</CardTitle>
            <CardDescription>
              Make changes to your account here. Click save when you're done.
            </CardDescription>
          </CardHeader>
          <CardContent className="space-y-2">
            {/* <div className="space-y-1">
              <Label htmlFor="name">Name</Label>
              <Input id="name" defaultValue="Pedro Duarte" />
            </div>
            <div className="space-y-1">
              <Label htmlFor="username">Username</Label>
              <Input id="username" defaultValue="@peduarte" />
            </div> */}
          </CardContent>
          <CardFooter>
            <Button>Save changes</Button>
          </CardFooter>
        </Card>
      </TabsContent>
      <TabsContent value="password">
        <Card>
          <CardHeader>
            <CardTitle>Password</CardTitle>
            <CardDescription>
              Change your password here. After saving, you'll be logged out.
            </CardDescription>
          </CardHeader>
          <CardContent className="space-y-2">
            {/* <div className="space-y-1">
              <Label htmlFor="current">Current password</Label>
              <Input id="current" type="password" />
            </div>
            <div className="space-y-1">
              <Label htmlFor="new">New password</Label>
              <Input id="new" type="password" />
            </div> */}
          </CardContent>
          <CardFooter>
            <Button>Save password</Button>
          </CardFooter>
        </Card>
      </TabsContent>
    </Tabs>
  )
}


export default DashBoard