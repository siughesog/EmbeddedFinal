import urequests, ujson
import xtools, utime
from machine import UART, Pin
import time

# 初始化 WiFi
xtools.connect_wifi_led()

# 城市列表
COUNTRY_CODE = "TW"
CITIES = [
    "Taipei",
    "New Taipei",
    "Taoyuan",
    "Taichung",
    "Tainan",
    "Kaohsiung",
    "Keelung",
    "Hsinchu",
    "Chiayi",
    "Hsinchu",
    "Miaoli",
    "Changhua",
    "Pingtung",
    "Yilan",
    "Hualien",
    "Taitung"
]


# 初始化 UART
uart = UART(2, baudrate=9600, tx=17, rx=16)

# URL encode
def url_encode(city_name):
    return city_name.replace(" ", "%20")

# 查天氣
def get_temperature_for_city(city_name, country_code):
    base_url = "https://api.openweathermap.org/data/2.5/weather?"
    query = "q=" + url_encode(city_name) + "," + country_code
    units = "&units=metric&lang=zh_tw"
    appid = "&appid=9ff2e87bf134ad54b2e067f9c68c0a3f"
    url = base_url + query + units + appid

    try:
        response = urequests.get(url)
        if response.status_code == 200:
            data = ujson.loads(response.text)
            temp = data.get("main", {}).get("temp")
            response.close()
            if temp is not None:
                print(f"{city_name} 溫度為 {temp:.2f}°C")
                return temp
        print(f"錯誤：{city_name} API 回傳狀態碼 {response.status_code}")
        response.close()
    except Exception as e:
        print("例外錯誤:", e)
    return None

# 溫度轉成 4 位數字（byte），例如 23.56 → [2, 3, 5, 6]
def float_to_digit_bytes(value):
    try:
        value_str = "{:.2f}".format(float(value)).replace(".", "")
        digits = [int(ch) for ch in value_str]
        return bytes(digits)
    except:
        return bytes([0, 0, 0, 0])

# 📌 主循環：不斷等待 index，查天氣，回傳，再等下一次
while True:
    if uart.any():
        incoming = uart.read(1)
        if incoming:
            selected_index = incoming[0]
            if 0 <= selected_index < len(CITIES):
                print(f"\n收到 index: {selected_index}")
                city = CITIES[selected_index]
                temperature = get_temperature_for_city(city, COUNTRY_CODE)

                if temperature is not None:
                    data = float_to_digit_bytes(temperature)
                    uart.write(data)
                    print(f"已發送 {city} 溫度：", data)
                else:
                    uart.write(bytes([0, 0, 0, 0]))
                    print(f"{city} 查詢失敗，發送空值")
            else:
                print(f"無效 index：{selected_index}")
    
    time.sleep(0.1)  # 小延遲避免 busy loop
