import tkinter as tk
from tkinter import ttk, messagebox
from tkcalendar import DateEntry
from datetime import datetime
import firebase_admin
from firebase_admin import credentials, db

# Firebase bağlantısı
cred = credentials.Certificate(r'C:\DeskPal\DeskPal Python\.json\deskpal-8e7a7-firebase-adminsdk-fbsvc-3dee1e76ba.json')
firebase_admin.initialize_app(cred, {
    'databaseURL': 'https://deskpal-8e7a7-default-rtdb.firebaseio.com/'
})

# Ana pencere
root = tk.Tk()
root.title("📅 DeskPal Ajanda Paneli")
root.geometry("600x500")
root.configure(bg="#f0f8ff")

# Başlık
header = tk.Label(root, text="📅 DeskPal Günlük Plan", font=("Arial", 16, "bold"), bg="#f0f8ff")
header.pack(pady=10)

# Plan Tablosu
columns = ("tarih", "zaman", "mod")
tree = ttk.Treeview(root, columns=columns, show="headings", height=10)
tree.heading("tarih", text="📆 Tarih")
tree.heading("zaman", text="⏰ Zaman")
tree.heading("mod", text="📚 Tür")
tree.pack(pady=10, padx=20, fill="both", expand=True)

# Plan Ekle Butonu
def open_add_plan_window():
    popup = tk.Toplevel(root)
    popup.title("Yeni Plan Ekle")
    popup.geometry("400x400")
    popup.configure(bg="#fffbe6")

    plan_type = tk.StringVar(value="ders")

    def choose_type(tip):
        plan_type.set(tip)
        if tip == "ders":
            ders_button.config(bg="#4a90e2", fg="white")
            mola_button.config(bg="lightgray", fg="black")
        else:
            mola_button.config(bg="#7ed321", fg="white")
            ders_button.config(bg="lightgray", fg="black")

    # Başlık
    tk.Label(popup, text="🎈 Yeni Plan Ekleyelim", font=("Comic Sans MS", 14, "bold"),
             bg="#fffbe6", fg="#ff6f00").pack(pady=(10, 0))

    # Tür Seçimi
    tk.Label(popup, text="🎯 Bu bir ders mi yoksa mola mı?", bg="#fffbe6", font=("Arial", 11, "bold")).pack()
    type_frame = tk.Frame(popup, bg="#f0fff0")
    type_frame.pack(pady=5)

    ders_button = tk.Button(type_frame, text="📘 Ders", width=10, bg="#4a90e2", fg="white",
                            font=("Arial", 11, "bold"), command=lambda: choose_type("ders"))
    ders_button.pack(side="left", padx=5)

    mola_button = tk.Button(type_frame, text="🛋️ Mola", width=10, bg="lightgray", fg="black",
                            font=("Arial", 11, "bold"), command=lambda: choose_type("mola"))
    mola_button.pack(side="left", padx=5)

    # Tarih
    tk.Label(popup, text="🗓️ Hangi Günde Yapacaksın?", bg="#fffbe6", font=("Arial", 11, "bold")).pack()
    date_entry = DateEntry(popup, date_pattern="yyyy-mm-dd", width=15, background='darkblue',
                           foreground='white', borderwidth=2, font=("Arial", 11))
    date_entry.pack(pady=5)

    # Saat Seçimi
    hours = [f"{i:02d}" for i in range(24)]
    minutes = [f"{i:02d}" for i in range(60)]

    tk.Label(popup, text="🕑 Başlangıç Saati", bg="#fffbe6", font=("Arial", 11, "bold")).pack()
    start_hour = tk.StringVar(value="16")
    start_minute = tk.StringVar(value="00")
    ttk.Combobox(popup, textvariable=start_hour, values=hours, width=5, state="readonly").pack()
    ttk.Combobox(popup, textvariable=start_minute, values=minutes, width=5, state="readonly").pack()

    tk.Label(popup, text="⏰ Bitiş Saati", bg="#fffbe6", font=("Arial", 11, "bold")).pack()
    end_hour = tk.StringVar(value="16")
    end_minute = tk.StringVar(value="30")
    ttk.Combobox(popup, textvariable=end_hour, values=hours, width=5, state="readonly").pack()
    ttk.Combobox(popup, textvariable=end_minute, values=minutes, width=5, state="readonly").pack()

    def kaydet_plan():
        tarih = date_entry.get()
        secilen_tarih = datetime.strptime(tarih, "%Y-%m-%d").date()
        bugun = datetime.today().date()
        tip = plan_type.get()

        plan_baslangic = datetime.strptime(f"{tarih} {start_hour.get()}:{start_minute.get()}", "%Y-%m-%d %H:%M")
        plan_bitis = datetime.strptime(f"{tarih} {end_hour.get()}:{end_minute.get()}", "%Y-%m-%d %H:%M")

        if secilen_tarih < bugun:
            messagebox.showerror("Hata", "Geçmiş tarihler için plan ekleyemezsiniz!")
            return

        if secilen_tarih == bugun:
            simdi = datetime.now()
            if plan_baslangic < simdi:
                messagebox.showerror("Hata", "Geçmiş saatler için plan ekleyemezsiniz!")
                return

        if plan_baslangic >= plan_bitis:
            messagebox.showerror("Hata", "Bitiş saati, başlangıç saatinden önce veya aynı olamaz!")
            return

        zaman_araligi = f"{start_hour.get()}:{start_minute.get()} - {end_hour.get()}:{end_minute.get()}"

        try:
            ref = db.reference(f"/schedule/{tarih}")
            ref.update({
                zaman_araligi: tip
            })
        except Exception as e:
            messagebox.showerror("Hata", f"Firebase'e yazılamadı:\n{e}")
            return

        tree.insert("", "end", values=(tarih, zaman_araligi, tip))
        popup.destroy()
        messagebox.showinfo("🎉 Başarılı", f"{tarih} günü için plan kaydedildi!")

    tk.Button(popup, text="✅ Kaydet ve Kapat", font=("Arial", 12, "bold"),
              bg="#ffd54f", fg="black", command=kaydet_plan).pack(pady=15)

# -- Buraya Düzenle ve Sil butonları ekliyoruz --

add_btn = tk.Button(root, text="➕ Plan Ekle", font=("Arial", 12, "bold"),
                    bg="#4CAF50", fg="white", command=open_add_plan_window)
add_btn.pack(pady=5, anchor="ne", padx=15)

edit_btn = tk.Button(root, text="✏️ Düzenle", font=("Arial", 12, "bold"),
                     bg="#2196F3", fg="white", command=lambda: edit_plan())
edit_btn.pack(pady=5, anchor="ne", padx=15)

delete_btn = tk.Button(root, text="🗑️ Sil", font=("Arial", 12, "bold"),
                       bg="#f44336", fg="white", command=lambda: delete_plan())
delete_btn.pack(pady=5, anchor="ne", padx=15)


# Seçilen planı alma
def get_selected_plan():
    selected = tree.selection()
    if not selected:
        messagebox.showwarning("Uyarı", "Lütfen düzenlemek veya silmek için bir plan seçin.")
        return None
    return selected[0]  # Seçili item ID'si

def get_plan_details(item_id):
    return tree.item(item_id, 'values')  # (tarih, zaman, mod)


# Silme fonksiyonu
def delete_plan():
    item_id = get_selected_plan()
    if not item_id:
        return
    cevap = messagebox.askyesno("Onay", "Seçili planı silmek istediğinize emin misiniz?")
    if not cevap:
        return

    tarih, zaman_araligi, tip = get_plan_details(item_id)
    try:
        ref = db.reference(f"/schedule/{tarih}/{zaman_araligi}")
        ref.delete()  # Firebase'den sil
    except Exception as e:
        messagebox.showerror("Hata", f"Firebase'den silinirken hata oluştu:\n{e}")
        return

    tree.delete(item_id)  # Listeden sil
    messagebox.showinfo("Başarılı", "Plan başarıyla silindi.")

# Düzenleme penceresi fonksiyonu
def edit_plan():
    item_id = get_selected_plan()
    if not item_id:
        return

    tarih, zaman_araligi, tip = get_plan_details(item_id)

    # Düzenleme penceresi
    popup = tk.Toplevel(root)
    popup.title("Planı Düzenle")
    popup.geometry("400x400")
    popup.configure(bg="#fffbe6")

    plan_type = tk.StringVar(value=tip)

    def choose_type(tip_sec):
        plan_type.set(tip_sec)
        if tip_sec == "ders":
            ders_button.config(bg="#4a90e2", fg="white")
            mola_button.config(bg="lightgray", fg="black")
        else:
            mola_button.config(bg="#7ed321", fg="white")
            ders_button.config(bg="lightgray", fg="black")

    # Başlık
    tk.Label(popup, text="📝 Plan Düzenle", font=("Comic Sans MS", 14, "bold"),
             bg="#fffbe6", fg="#ff6f00").pack(pady=(10, 0))

    # Tür Seçimi
    tk.Label(popup, text="🎯 Bu bir ders mi yoksa mola mı?", bg="#fffbe6", font=("Arial", 11, "bold")).pack()
    type_frame = tk.Frame(popup, bg="#f0fff0")
    type_frame.pack(pady=5)

    ders_button = tk.Button(type_frame, text="📘 Ders", width=10, font=("Arial", 11, "bold"),
                            command=lambda: choose_type("ders"))
    mola_button = tk.Button(type_frame, text="🛋️ Mola", width=10, font=("Arial", 11, "bold"),
                            command=lambda: choose_type("mola"))
    ders_button.pack(side="left", padx=5)
    mola_button.pack(side="left", padx=5)

    choose_type(tip)  # Buton renklerini ilk duruma göre ayarla

    # Tarih
    tk.Label(popup, text="🗓️ Hangi Günde Yapacaksın?", bg="#fffbe6", font=("Arial", 11, "bold")).pack()
    date_entry = DateEntry(popup, date_pattern="yyyy-mm-dd", width=15, background='darkblue',
                           foreground='white', borderwidth=2, font=("Arial", 11))
    date_entry.pack(pady=5)
    date_entry.set_date(datetime.strptime(tarih, "%Y-%m-%d").date())

    # Saat Seçimi
    hours = [f"{i:02d}" for i in range(24)]
    minutes = [f"{i:02d}" for i in range(0, 60)]

    tk.Label(popup, text="🕑 Başlangıç Saati", bg="#fffbe6", font=("Arial", 11, "bold")).pack()
    start_hour = tk.StringVar(value=zaman_araligi.split(" - ")[0].split(":")[0])
    start_minute = tk.StringVar(value=zaman_araligi.split(" - ")[0].split(":")[1])
    ttk.Combobox(popup, textvariable=start_hour, values=hours, width=5, state="readonly").pack()
    ttk.Combobox(popup, textvariable=start_minute, values=minutes, width=5, state="readonly").pack()

    tk.Label(popup, text="⏰ Bitiş Saati", bg="#fffbe6", font=("Arial", 11, "bold")).pack()
    end_hour = tk.StringVar(value=zaman_araligi.split(" - ")[1].split(":")[0])
    end_minute = tk.StringVar(value=zaman_araligi.split(" - ")[1].split(":")[1])
    ttk.Combobox(popup, textvariable=end_hour, values=hours, width=5, state="readonly").pack()
    ttk.Combobox(popup, textvariable=end_minute, values=minutes, width=5, state="readonly").pack()

    # Düzenleme Kaydetme Fonksiyonu
    def kaydet_duzenleme():
        yeni_tarih = date_entry.get()
        secilen_tarih = datetime.strptime(yeni_tarih, "%Y-%m-%d").date()
        bugun = datetime.today().date()
        yeni_tip = plan_type.get()

        yeni_baslangic = datetime.strptime(f"{yeni_tarih} {start_hour.get()}:{start_minute.get()}", "%Y-%m-%d %H:%M")
        yeni_bitis = datetime.strptime(f"{yeni_tarih} {end_hour.get()}:{end_minute.get()}", "%Y-%m-%d %H:%M")

        if secilen_tarih < bugun:
            messagebox.showerror("Hata", "Geçmiş tarihler için plan ekleyemezsiniz!")
            return

        if secilen_tarih == bugun:
            simdi = datetime.now()
            if yeni_baslangic < simdi:
                messagebox.showerror("Hata", "Geçmiş saatler için plan ekleyemezsiniz!")
                return

        if yeni_baslangic >= yeni_bitis:
            messagebox.showerror("Hata", "Bitiş saati, başlangıç saatinden önce veya aynı olamaz!")
            return

        yeni_zaman_araligi = f"{start_hour.get()}:{start_minute.get()} - {end_hour.get()}:{end_minute.get()}"

        try:
            # Önce eski planı sil
            eski_ref = db.reference(f"/schedule/{tarih}/{zaman_araligi}")
            eski_ref.delete()

            # Yeni planı ekle
            yeni_ref = db.reference(f"/schedule/{yeni_tarih}")
            yeni_ref.update({
                yeni_zaman_araligi: yeni_tip
            })
        except Exception as e:
            messagebox.showerror("Hata", f"Firebase'e yazılamadı:\n{e}")
            return

        # Ağaç görünümünde güncelleme
        tree.item(item_id, values=(yeni_tarih, yeni_zaman_araligi, yeni_tip))

        popup.destroy()
        messagebox.showinfo("🎉 Başarılı", "Plan başarıyla güncellendi!")

    tk.Button(popup, text="💾 Kaydet ve Kapat", font=("Arial", 12, "bold"),
              bg="#ffd54f", fg="black", command=kaydet_duzenleme).pack(pady=15)


# Firebase'den kayıtları çekip Treeview'a yükleme
def load_plans_from_firebase():
    try:
        ref = db.reference("/schedule")
        data = ref.get()
        if data:
            for tarih, planlar in data.items():
                for zaman_araligi, tip in planlar.items():
                    tree.insert("", "end", values=(tarih, zaman_araligi, tip))
    except Exception as e:
        messagebox.showerror("Hata", f"Veriler yüklenirken hata oluştu:\n{e}")

# Program açılır açılmaz planları yükle
load_plans_from_firebase()

# Başlat
root.mainloop()
