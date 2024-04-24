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
import Switch from "react-switch";
import {
  Tabs,
  TabsContent,
  TabsList,
  TabsTrigger,
} from "@/components/ui/tabs"

import Paho from 'paho-mqtt'
import { useState,useEffect } from "react"
import Gauge from "@/components/gauge";

const DashBoard = () => {
  // let client;
  const [client, setClient] = useState(null)
  const [isConnected, setIsConnected] = useState(false);
  const [turbineWorking, setTurbineWorking] = useState(false);
  const [temperature, setTemperature] = useState(0);
  const [humidity, setHumidity] = useState(0);
  const [voltage, setVoltage] = useState(0);
  const [current, setCurrent] = useState(0);
  const [acceleration, setAcceleration] = useState([0,0,0]);
  const [freq, setFreq] = useState(0)

  const connectMqTT = () => {
    
    const mqttclient = new Paho.Client("broker.hivemq.com", Number(8000), "sdkfhbuvsd vsdvcsuwdfwv sdhcs 123e2edsa121245vsdverr4tw32");
    
    console.log('connecting');

    mqttclient.connect({
      onSuccess: () => {
        console.log("onConnect");
        mqttclient.subscribe("esp32/pub");

        mqttclient.onMessageArrived = (message) => {
          let responseObject = JSON.parse(message.payloadString)
          console.log(responseObject);
          // console.log(message.payloadString);
          setTemperature(responseObject.temperature)
          setHumidity(responseObject.humidity)
          setVoltage(responseObject.voltage)
          setCurrent(responseObject.current)
          setAcceleration(responseObject.Vibration.acceleration)
          setFreq(responseObject.frequency)
        }
        mqttclient.onConnectionLost = (responseObject) => {
          console.log(responseObject);
        }
        
        console.log('connected');
        setClient(mqttclient);
        setIsConnected(true);
      }, 
      onFailure:(error) => {console.log(error);} }); 
  }

  const changeTurbineState = () => {
    if (!turbineWorking){
      let message = new Paho.Message("off");
      message.destinationName = "esp32/sub";
      client.send(message);
      console.log(message, "off com");
    }else{
      let message = new Paho.Message("on");
      message.destinationName = "esp32/sub";
      client.send(message);
      console.log(message, "on com");

    }
    setTurbineWorking(prev => !prev);
  }

  useEffect(()=>{
    if(!isConnected){
      connectMqTT();
    }
  },[])


  return (
    <Tabs defaultValue="current" className="w-3/4 mx-auto">
      <TabsList className="grid grid-cols-2 mx-auto">
        <TabsTrigger value="current">Current</TabsTrigger>
        <TabsTrigger value="historical">Historical</TabsTrigger>
      </TabsList>
      <TabsContent value="current">
        <Card>
          <CardHeader>
            <div className="flex items-center">
              <CardTitle >Real time data   </CardTitle>
              <CardDescription>
                Real time data visualization of wind turbine parameters 
              </CardDescription>
            </div>
          </CardHeader>
          <CardContent className="space-y-2">
            <div className="flex gap-6 ">
              <div className="bg-blue-200  px-8 rounded-md">
                <h3 className="text-2xl font-medium text-center pt-4">Environmental Parameters</h3>
                <div className="flex">
                  <Gauge title={"Temperature"} value={temperature}/>
                  <Gauge title={"Humidity"} value={humidity}/>
                </div>
              </div>

              <div className="bg-blue-200 px-6 rounded-md overflow-clip flex flex-col flex-1">
                <h3 className="text-xl font-medium text-center">Turbine state</h3>
                <div className="flex-1 flex flex-col items-center justify-center">
                  {/* <span>Switch turbine state</span> */}
                  <div className="">
                    <Switch className="" onChange={changeTurbineState} checked={turbineWorking} />
                  </div>
                </div>
              </div>
            </div>
            <div className="flex gap-6">
              <div className="bg-blue-200  px-4 rounded-md basis-1/3">
                <h3 className="text-2xl font-medium text-center pt-4">Electrical Parameters</h3>
                <div className="flex flex-col">
                  <Gauge title={"Voltage"} value={voltage}/>
                  <Gauge title={"Current"} value={current}/>
                </div>
              </div>
              <div className="bg-blue-200  px-4 rounded-md ">
                <h3 className="text-2xl font-medium text-center pt-4">Mechanical Parameters</h3>
                <div className="flex">
                  <Gauge title={"Acceleration X axis"} value={acceleration[0]} width={300} height={250}/>
                  <Gauge title={"Acceleration y axis"} value={acceleration[1]} width={300}/>
                </div>
                <div className="flex">
                  <Gauge title={"Acceleration Z axis"} value={acceleration[2]} width={300} height={250}/>
                  <Gauge title={"Vibration"} value={freq} width={300}/>
                </div>
              </div>

            </div>

          </CardContent>
        </Card>
      </TabsContent>
      <TabsContent value="password">
        <Card>
          <CardHeader>
            <CardTitle>Password</CardTitle>
            <CardDescription>
              Change your password here. After saving, youll be logged out.
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