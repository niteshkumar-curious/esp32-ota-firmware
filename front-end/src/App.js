import { useEffect, useState } from "react"
import axios from "axios"

function App() {

    const API = "https://backend-ota.onrender.com"

    const [led, setLed] = useState("0")

    const [device, setDevice] = useState("offline")

    const [otaProgress, setOtaProgress] = useState(0)

    const [otaResult, setOtaResult] = useState("")

    const [updating, setUpdating] = useState(false)

    const [currentVersion, setCurrentVersion] = useState("unknown")

    const [firmwares, setFirmwares] = useState([])

    const [selectedVersion, setSelectedVersion] = useState("")

    const [temperature, setTemperature] = useState(null)

    const [rollbackRunning, setRollbackRunning] = useState(false)

    /* ================= LOAD DEVICE STATUS ================= */

    const loadStatus = async() => {

        try {

            const res = await axios.get(API + "/status")

            setLed(res.data.led)

            setDevice(res.data.device)

            setCurrentVersion(res.data.version)

            if (res.data.temperature !== undefined) {

                setTemperature(res.data.temperature)

            } else {

                setTemperature(null)

            }

            if (res.data.ota !== undefined) {

                setOtaProgress(res.data.ota)
            }

            if (res.data.otaResult) {

                setOtaResult(res.data.otaResult)

                if (
                    res.data.otaResult === "success" ||
                    res.data.otaResult === "failed"
                ) {

                    setUpdating(false)

                    setRollbackRunning(false)

                }
            }

        } catch {

            setDevice("offline")
        }
    }

    /* ================= LOAD FIRMWARE LIST ================= */

    const loadFirmwares = async() => {

        try {

            const res = await axios.get(API + "/firmwares")

            setFirmwares(res.data)

            if (res.data.length > 0) {

                setSelectedVersion(res.data[0].version)
            }

        } catch {}
    }

    /* ================= INITIAL LOAD ================= */

    useEffect(() => {

        loadStatus()

        loadFirmwares()

        const interval = setInterval(loadStatus, 2000)

        return () => clearInterval(interval)

    }, [])

    /* ================= TOGGLE LED ================= */

    const toggleLed = async() => {

        if (device !== "online") return

        const newState = led === "1" ? "0" : "1"

        try {

            const res = await axios.post(
                API + "/led", {
                    state: newState
                }
            )

            if (res.data.success) {

                setLed(newState)
            }

        } catch {}
    }

    /* ================= OTA UPDATE ================= */

    const ota = async() => {

        if (device !== "online") return

        setUpdating(true)

        setOtaProgress(0)

        setOtaResult("")

        try {

            await axios.post(
                API + "/ota", {
                    version: selectedVersion
                }
            )

        } catch {}
    }

    /* ================= ROLLBACK ================= */

    const rollback = async() => {

        if (device !== "online") return

        setUpdating(true)

        setRollbackRunning(true)

        setOtaProgress(0)

        setOtaResult("")

        try {

            await axios.post(API + "/rollback")

        } catch {}
    }

    return (

        <
        div style = { styles.container } >

        { /* TITLE */ }

        <
        h1 style = { styles.title } >
        ESP32 IoT Dashboard <
        /h1>

        { /* DEVICE STATUS */ }

        <
        div style = { styles.card } >

        <
        h2 >

        Device Status:

        <
        span style = {
            {
                color: device === "online" ?
                    "#22c55e" : "#ef4444",

                marginLeft: 10
            }
        } >

        { device }

        <
        /span>

        <
        /h2>

        <
        h3 >

        Firmware: { " " } { currentVersion }

        <
        /h3>

        <
        /div>

        {
            currentVersion === "v1.0.4" && (

                <
                div style = { styles.card } >

                <
                h2 > Temperature < /h2>

                <
                h1 >

                {
                    temperature !== null ?

                    temperature.toFixed(2) + " °C"

                    :

                        "--"

                }

                <
                /h1>

                <
                /div>

            )
        }


        { /* LED CONTROL */ }

        <
        div style = { styles.card } >

        <
        h2 >

        LED Status: { " " } { led === "1" ? "ON" : "OFF" }

        <
        /h2>

        <
        button

        style = {
            {

                ...styles.button,

                    opacity:
                    device !== "online" ?
                    0.5 :
                    1,

                    cursor:
                    device !== "online" ?
                    "not-allowed" :
                    "pointer"
            }
        }

        onClick = { toggleLed }

        disabled = { device !== "online" }

        >

        Toggle LED

        <
        /button>

        <
        /div>

        { /* OTA */ }

        <
        div style = { styles.card } >

        <
        h2 >

        OTA Firmware Update

        <
        /h2>

        { /* FIRMWARE SELECT */ }

        <
        select

        style = { styles.select }

        value = { selectedVersion }

        onChange = {
            (e) =>
            setSelectedVersion(e.target.value)
        }

        disabled = { device !== "online" }

        >

        {
            firmwares.map((fw, index) => (

                <
                option

                key = { index }

                value = { fw.version }

                disabled = { fw.version === currentVersion }

                >

                {
                    fw.version === currentVersion

                    ?

                    fw.version + " (Current)"

                    :

                        fw.version

                }

                <
                /option>

            ))
        }

        <
        /select>

        {
            selectedVersion === currentVersion && (

                <
                p style = {
                    { color: "#facc15" }
                } >

                Current firmware already installed.

                <
                /p>

            )
        }


        { /* OTA BUTTON */ }

        <
        button

        style = {
            {

                ...styles.otaButton,

                    opacity:

                    device !== "online" ||

                    selectedVersion === currentVersion

                    ?

                    0.5

                    :

                    1,

                    cursor:
                    device !== "online" ?
                    "not-allowed" :
                    "pointer"
            }
        }

        onClick = { ota }

        disabled = {

            device !== "online" ||

            selectedVersion === currentVersion

        }

        >

        Start OTA

        <
        /button>

        { /* ROLLBACK BUTTON */ }

        <
        button

        style = {
            {

                ...styles.rollbackButton,

                    opacity:
                    device !== "online" ?
                    0.5 :
                    1,

                    cursor:
                    device !== "online" ?
                    "not-allowed" :
                    "pointer"
            }
        }

        onClick = { rollback }

        disabled = { device !== "online" }

        >

        Rollback

        <
        /button>

        {
            rollbackRunning && (

                <
                p style = {
                    {
                        color: "#f59e0b"
                    }
                } >

                Rollback Started...

                <
                /p>

            )
        }

        { /* OTA PROGRESS */ }

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
                p >

                { otaProgress } %

                <
                /p>

                <
                /div>

            )
        }

        { /* OTA RESULT */ }

        {
            otaResult === "success" && (

                <
                p style = {
                    {
                        color: "#22c55e"
                    }
                } >

                {
                    rollbackRunning

                    ?

                    "Rollback Successful"

                    :

                        "Update Successful"

                }

                <
                /p>

            )
        }

        {
            otaResult === "failed" && (

                <
                p style = {
                    {
                        color: "#ef4444"
                    }
                } >

                Update Failed

                <
                /p>

            )
        }

        <
        /div>

        <
        /div>
    )
}

/* ================= STYLES ================= */

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

        width: "340px",

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

        fontSize: "16px"
    },

    otaButton: {

        marginTop: "15px",

        padding: "10px 25px",

        border: "none",

        borderRadius: "6px",

        background: "#22c55e",

        color: "white",

        fontSize: "16px"
    },

    rollbackButton: {

        marginTop: "15px",

        marginLeft: "10px",

        padding: "10px 25px",

        border: "none",

        borderRadius: "6px",

        background: "#f59e0b",

        color: "white",

        fontSize: "16px"
    },

    select: {

        width: "100%",

        padding: "10px",

        marginTop: "10px",

        borderRadius: "6px",

        fontSize: "16px"
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