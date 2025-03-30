
import tkinter as tk
from tkinter import messagebox
import random
import time
import serial

# UART için bir seri port objesi oluştur

uart = serial.Serial(port="COM3", baudrate=9600, timeout=1)
print(uart.name)


# Türkçe'de yaygın kullanılan 100 kelime
word_list = [
    "ev", "ağaç", "kitap", "okul", "bilgisayar", "yazılım", "çocuk", "arkadaş", "bahçe", "saat",
    "kalem", "defter", "telefon", "kamera", "kedi", "köpek", "masa", "sandık", "bardak", "yastık",
    "yemek", "televizyon", "araba", "bisiklet", "uçak", "yazı", "resim", "renk", "ışık", "şehir",
    "köy", "dağ", "deniz", "nehir", "ırmak", "çay", "kahve", "ekmek", "peynir", "zeytin",
    "elbise", "şapka", "ayakkabı", "çorap", "mont", "pencere", "kapı", "dolap", "halı", "perde",
    "yatak", "çarşaf", "sandalye", "kitaplık", "raf", "klima", "fare", "uçurtma", "gitar", "piyano",
    "sinema", "tiyatro", "kütüphane", "müzik", "dans", "oyun", "kamera", "taksi", "otobüs", "tren",
    "istasyon", "havaalanı", "yol", "kaldırım", "park", "bahçe", "orman", "çiçek", "yıldız", "güneş",
    "ay", "bulut", "yağmur", "kar", "rüzgar", "fırtına", "sıcaklık", "soğuk", "temizlik", "alışveriş",
    "hediye", "doğum", "tatil", "okul", "sınav", "ders", "öğretmen", "öğrenci", "kalem", "silgi"
]

# Doğru kelime sayısını tutan değişken
correct_words = 0
remaining_time = 30  # Süreyi 60 saniye olarak ayarlayın


def start_test():
    """Testi başlatır."""
    global correct_words, remaining_time

    correct_words = 0
    remaining_time = 30
    entry_box.config(state="normal")  # Giriş kutusunu etkinleştir
    word_label.config(text=random.choice(word_list))

    # OLED'e "begin" parametresini gönder
    if uart.is_open:
        uart.write(b'a')  # "begin" komutunu gönder

    """ Displayi 60'dan geriye doğru saymaya başlatmak ve OLED ekrana başlama emrini verir"""
    start_timer()


def start_timer():
    """Zamanlayıcıyı başlatır ve geri sayımı yapar."""
    global remaining_time
    if remaining_time > 0:
        remaining_time -= 1
        timer_label.config(text=f"Kalan Süre: {remaining_time} saniye")
        window.after(1000, start_timer)  # Her saniye bu fonksiyonu tekrar çağır
    else:
        entry_box.config(state="disabled")  # Giriş kutusunu devre dışı bırak
        """messagebox.showinfo("Süre Doldu!", f"Doğru Kelime Sayısı: {correct_words}")"""
        print(f"Doğru Kelime Sayısı: {correct_words}")  # Terminale yazdır

        # UART üzerinden OLED'e doğru kelime sayısını gönder
        if uart.is_open:
            correct_words_str = str(correct_words)  # String'e çevir
            time.sleep(1)
            uart.write(correct_words_str.encode() + b'')  # UART üzerinden gönder


def check_word(event=None):
    """Kullanıcının girdiği kelimeyi kontrol eder."""
    global correct_words
    user_input = entry_box.get().strip()
    current_word = word_label.cget("text")

    if user_input == current_word:
        correct_words += 1

    # Yeni kelimeyi göster ve giriş kutusunu temizle
    word_label.config(text=random.choice(word_list))
    entry_box.delete(0, tk.END)


# Pencereyi oluştur
window = tk.Tk()
window.title("Yazma Hız Testi")
window.geometry("400x200")
window.resizable(False, False)

# Üstteki kelimeyi göstermek için bir etiket
word_label = tk.Label(window, text="", font=("Arial", 24))
word_label.pack(pady=20)

# Kalan süreyi göstermek için bir etiket
timer_label = tk.Label(window, text=f"Kalan Süre: {remaining_time} saniye", font=("Arial", 14))
timer_label.pack()

# Giriş kutusu
entry_box = tk.Entry(window, font=("Arial", 18))
entry_box.pack(pady=10)
entry_box.bind("<Return>", check_word)
entry_box.config(state="disabled")  # Başlangıçta devre dışı bırak

# Başlat Butonu
start_button = tk.Button(window, text="Testi Başlat", font=("Arial", 14), command=start_test)
start_button.pack(pady=10)

# Pencereyi çalıştır
window.mainloop()