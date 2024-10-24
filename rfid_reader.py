import time
import serial
import serial.tools.list_ports
from plyer import notification
import webbrowser
import requests

# Function to find Arduino connected on a COM port
def find_arduino():
    """Find Arduino by checking available COM ports."""
    ports = serial.tools.list_ports.comports()
    for port in ports:
        if 'Arduino' in port.description or 'CH340' in port.description:
            return port.device  # Return the COM port (e.g., 'COM7')
    return None

# Function to show a pop-up notification when the Arduino is connected
def show_popup(port):
    """Display a pop-up notification when the Arduino is connected."""
    notification.notify(
        title="Arduino Connected",
        message=f"Arduino detected on {port}. You can now scan RFID cards.",
        app_icon=None,  # Optional: Provide path to .ico file for custom icon
        timeout=10  # Notification will last for 10 seconds
    )

# Function to check RFID data with the server and return total row
def check_rfid(rfid_data):
    """Check the RFID data with the server and return total row."""
    url = f"https://bus-arka-server.vercel.app/api/check-rfid?rfid={rfid_data}"
    try:
        response = requests.get(url)
        response.raise_for_status()  # Raise an error for bad responses
        return response.json()  # Return the JSON response
    except requests.exceptions.RequestException as e:
        print(f"Error checking RFID: {e}")
        return None  # Return None if there's an error

# Function to handle the RFID response and redirect accordingly
def handle_rfid_response(rfid_response):
    """Handle the response from the RFID check and redirect accordingly."""
    if rfid_response and 'totalrow' in rfid_response:
        total_row = rfid_response['totalrow']
        if total_row == 0:
            # Open registration link if no rows found
            print(f"Registering new RFID: {rfid_response['rfid']}")
            register_rfid(rfid_response['rfid'])
        else:
            # Open error page if data exists
            error_url = "https://bus-arka.vercel.app/error"
            webbrowser.open(error_url)
            print(f"Opening error link: {error_url}")
    else:
        print("Invalid response structure or empty response.")

# Function to open registration URL in the default web browser
def register_rfid(rfid_data=''):
    """Open the registration URL in the default web browser with RFID data."""
    url = f"https://bus-arka.vercel.app/admin/registration?rfid={rfid_data}"
    print(f"Opening registration URL: {url}")
    webbrowser.open(url)

# Function to read RFID data from the Arduino serial input
def read_rfid(ser):
    """Read RFID data from the Arduino serial input."""
    while True:
        if ser.in_waiting > 0:
            rfid_data_string = ser.readline().decode('utf-8').strip()

            # Debug: Print the raw data received
            print(f"Raw RFID data received: {rfid_data_string}")
            
            rfid_data = ''
            # Check if the string starts with "RFID Data" (you can change this to match your exact RFID format)
            if rfid_data_string.startswith("RFID Data"):
                try:
                    # Extract the actual RFID data and convert it to lowercase
                    rfid_data = rfid_data_string.split(": ")[1].lower()  # Get the second part after splitting
                    print(f"Extracted RFID: {rfid_data}")  # Output the RFID data
                    
                    # Check the RFID with the server
                    rfid_response = check_rfid(rfid_data)  # Check RFID
                    handle_rfid_response(rfid_response)  # Handle the response
                except IndexError:
                    print("Error parsing RFID data, format incorrect.")
            else:
                print("No valid RFID scan detected. Waiting for next scan.")
            
            time.sleep(1)

# Main loop to detect Arduino connection and read RFID
if __name__ == "__main__":
    print("Searching for Arduino...")
    arduino_detected = False

    while True:
        # Detect Arduino connection
        port = find_arduino()
        if port and not arduino_detected:
            print(f"Arduino found on {port}")
            show_popup(port)  # Show pop-up notification
            arduino_detected = True  # Set flag to prevent multiple pop-ups

            # Set up serial communication once Arduino is detected
            try:
                ser = serial.Serial(port, 9600)  # Adjust baud rate if necessary
                print("Serial communication established. Waiting for RFID scan...")

                # Start reading RFID data
                read_rfid(ser)
            except serial.SerialException as e:
                print(f"Error opening port {port}: {e}")
        
        elif not port and arduino_detected:
            print("Arduino disconnected.")
            arduino_detected = False  # Reset flag when Arduino is disconnected

        elif port:
            print("Arduino is connected but no RFID scan yet. Waiting...")
        
        else:
            print("Arduino not found. Retrying in 5 seconds...")
        
        time.sleep(5)  # Wait 5 seconds before retrying
