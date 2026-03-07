import { useEffect, useState } from "react"
import axios from "axios"

function App() {

    const API = "https://backend-ota.onrender.com"

    const [led, setLed] = useState(0)
    const [device, setDevice] = useState("offline")

    const loadStatus = async() => {

        const res = await axios.get(API + "/status")

        setLed(res.data.led)
        setDevice(res.data.device)

    }

    useEffect(() => {
        loadStatus()
        setInterval(loadStatus, 2000)
    }, [])

    const toggleLed = () => {

        const newState = led == "1" ? "0" : "1"

        axios.post(API + "/led", { state: newState })
    }

    const ota = () => {

        axios.post(API + "/ota")
    }

    return (

        <
        div style = {
            { textAlign: "center" }
        } >

        <
        h1 > ESP32 Dashboard < /h1>

        <
        h2 > Device Status: { device } < /h2>

        <
        h2 > LED Status: { led } < /h2>

        <
        button onClick = { toggleLed } > Toggle LED < /button>

        <
        br / > < br / >

        <
        button onClick = { ota } > OTA Update < /button>

        <
        /div>
    )

}

export default App