'use client'
import { useEffect, useState } from "react";
import DashBoard from "./DashBoard";
import Paho from "paho-mqtt"

export default function Home() {
  
  return (
    // <main className="flex min-h-screen flex-col items-center justify-between p-24">
    // </main>
    <div className="bg-gradient-to-br from-blue-200 to-cyan-200 min-h-screen">
      <header>
        <h1 className="text-blue-700 p-4 text-4xl font-semibold text-center">
          Windtor - Monitoring Platform For Wind Turbines
        </h1>
      </header>
      <main>
        <DashBoard />
      </main>
      <footer>

      </footer>
    </div>
  );
}
