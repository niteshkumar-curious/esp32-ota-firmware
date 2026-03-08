import { useEffect, useState } from "react"
import axios from "axios"

function App() {

    const API = "https://backend-ota.onrender.com" // put your Render backend URL

    const [led, setLed] = useState("0")
    const [device, setDevice] = useState("offline")
    const [otaProgress, setOtaProgress] = useState(0)
    const [updating, setUpdating] = useState(false)

    const loadStatus = async() => {

        try {
            const res = await axios.get(API + "/status")

            setLed(res.data.led)
            setDevice(res.data.device)

            if (res.data.ota) {
                setOtaProgress(res.data.ota)
            }

        } catch {
            setDevice("offline")
        }
    }

    useEffect(() => {

        loadStatus()

        const interval = setInterval(loadStatus, 2000)

        return () => clearInterval(interval)

    }, [])

    const toggleLed = async() => {

        const newState = led === "1" ? "0" : "1"

        await axios.post(API + "/led", { state: newState })

        setLed(newState)
    }

    const ota = async() => {

        setUpdating(true)
        setOtaProgress(0)

        await axios.post(API + "/ota")
    }

    return (

        <
        div style = { styles.container } >

        <
        h1 style = { styles.title } > ESP32 IoT Dashboard < /h1>

        <
        div style = { styles.card } >

        <
        h2 >
        Device Status:
        <
        span style = {
            {
                color: device === "online" ? "#2ecc71" : "#e74c3c",
                marginLeft: 10
            }
        } > { device } <
        /span> <
        /h2>

        <
        /div>

        <
        div style = { styles.card } >

        <
        h2 > LED Status: { led === "1" ? "ON" : "OFF" } < /h2>

        <
        button style = { styles.button }
        onClick = { toggleLed } >
        Toggle LED <
        /button>

        <
        /div>

        <
        div style = { styles.card } >

        <
        h2 > OTA Firmware Update < /h2>

        <
        button style = { styles.otaButton }
        onClick = { ota } >
        Start OTA <
        /button>

        {
            updating && (

                <
                div style = { styles.progressContainer } >

                <
                div style = {
                    {
                        ...styles.progressBar,
                            width: otaProgress + "%"
                    }
                }
                />

                <
                p > { otaProgress } % < /p>

                <
                /div>

            )
        }

        <
        /div>

        <
        /div>
    )
}

const styles = {

    container: {
        fontFamily: "Arial",
        textAlign: "center",
        background: "#0f172a",
        minHeight: "100vh",
        color: "white",
        paddingTop: "40px"
    },

    title: {
        marginBottom: "30px"
    },

    card: {
        background: "#1e293b",
        margin: "20px auto",
        padding: "25px",
        width: "320px",
        borderRadius: "10px",
        boxShadow: "0 5px 15px rgba(0,0,0,0.3)"
    },

    button: {
        marginTop: "10px",
        padding: "10px 25px",
        border: "none",
        borderRadius: "6px",
        background: "#3b82f6",
        color: "white",
        fontSize: "16px",
        cursor: "pointer"
    },

    otaButton: {
        marginTop: "10px",
        padding: "10px 25px",
        border: "none",
        borderRadius: "6px",
        background: "#22c55e",
        color: "white",
        fontSize: "16px",
        cursor: "pointer"
    },

    progressContainer: {
        marginTop: "20px"
    },

    progressBar: {
        height: "12px",
        background: "#22c55e",
        borderRadius: "6px",
        transition: "0.4s"
    }
}

export default App