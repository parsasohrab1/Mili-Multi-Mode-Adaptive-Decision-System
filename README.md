# Mili-Multi-Mode-Adaptive-Decision-System

سامانه تصمیم‌گیری تطبیقی چندحالته
Multi-Mode Adaptive Decision System
1. مقدمه (Introduction)
1.1 هدف (Purpose)
این سند مشخصات کامل سامانه تصمیم‌گیری هوشمند برای انتخاب خودکار بهترین حالت عملیاتی پهپاد بر اساس شرایط محیطی و مأموریت را تشریح می‌کند.

1.2 محدوده (Scope)
۶ حالت عملیاتی: شناسایی، نظارت، تعقیب، گشت‌زنی، بازگشت، درگیری

تصمیم‌گیری مبتنی بر ۵ مؤلفه: باتری، تهدید، ارتباط، دید، اولویت

بهینه‌سازی همزمان انرژی، موفقیت مأموریت و بقا

نرخ به‌روزرسانی: ۱۰ هرتز

1.3 اصطلاحات و اختصارات
اختصار	توضیح
MCDM	Multi-Criteria Decision Making
TOPSIS	Technique for Order of Preference by Similarity to Ideal Solution
FSM	Finite State Machine - ماشین حالت محدود
QoS	Quality of Service - کیفیت سرویس
2. الزامات کلی
2.1 پرسپکتیو محصول
این سامانه به عنوان لایه تصمیم‌گیرنده بر روی پردازنده اصلی (STM32H7) اجرا می‌شود و از داده‌های حسگرها و محصولات ۱ و ۲ استفاده می‌کند.

text
┌─────────────────────────────────────────────────┐
│         سامانه تصمیم‌گیری تطبیقی               │
├─────────────────────────────────────────────────┤
│  ورودی‌ها:                                      │
│  • وضعیت باتری (محصول ۱ - مدیریت توان)         │
│  • موقعیت/ناوبری (محصول ۲ - VIO)              │
│  • داده‌های حسگر محیطی                         │
│  • دستورات اپراتور                             │
├─────────────────────────────────────────────────┤
│  هسته تصمیم‌گیری:                              │
│  • محاسبه امتیاز ۶ حالت                        │
│  • بهینه‌سازی چندهدفه                          │
│  • انتخاب حالت برتر                            │
├─────────────────────────────────────────────────┤
│  خروجی:                                         │
│  • حالت عملیاتی انتخاب‌شده                     │
│  • پارامترهای کنترل حالت                       │
│  • گزارش‌های تصمیم‌گیری                        │
└─────────────────────────────────────────────────┘
2.2 ویژگی‌های اصلی
ارزیابی چندمعیاره: ترکیب ۵ مؤلفه با وزن‌دهی پویا

بهینه‌سازی چندهدفه: تعادل بین انرژی، موفقیت و بقا

تطبیق‌پذیری: تنظیم وزن‌ها بر اساس اولویت مأموریت

مقاوم‌سازی: تصمیم‌گیری در شرایط داده‌های ناقص

3. الزامات سیستم
3.1 الزامات سخت‌افزاری
مؤلفه	مشخصات
پردازنده	STM32H7 (480 MHz)
حافظه	۵۱۲ کیلوبایت رم
ورودی‌ها	CAN, UART, SPI
3.2 الزامات نرم‌افزاری
پیاده‌سازی به زبان C++ با قابلیت بازپیکربندی

جدول وزن‌دهی قابل تنظیم در حین اجرا

لاگ‌گیری تمام تصمیم‌ها برای تحلیل پس‌وقوع

4. مشخصات عملکردی
4.1 FR-1: محاسبه امتیاز حالت‌ها
توضیح: محاسبه امتیاز هر یک از ۶ حالت بر اساس شرایط فعلی.

فرمول امتیازدهی (بر اساس داده‌های سنتتیک):

text
Score(mode) = Σ(w_i * normalized_factor_i)
معیار پذیرش:

زمان محاسبه < ۵ms

صحت انتخاب حالت > ۹۲% در شبیه‌سازی

4.2 FR-2: انتخاب حالت بهینه
توضیح: انتخاب حالتی با بالاترین امتیاز و اعمال آن به سیستم کنترل.

معیار پذیرش:

زمان سوئیچ حالت < ۵۰ms

جلوگیری از نوسان (hysteresis) با تأخیر ۵ ثانیه

4.3 FR-3: گزارش‌گیری و تحلیل
توضیح: ثبت تمام تصمیم‌ها برای تحلیل و بهبود الگوریتم.

معیار پذیرش:

ذخیره ۱۰۰۰ تصمیم آخر

خروجی قابل خواندن برای تحلیلگر

5. کد تولید داده - محصول سوم
python
# =====================================================
# SRS - PRODUCT 3: MULTI-MODE ADAPTIVE DECISION SYSTEM
# =====================================================

import numpy as np
import pandas as pd
from sklearn.preprocessing import StandardScaler

np.random.seed(2026)

def generate_mission_decision_data():
    """
    تولید داده‌های سناریوهای مأموریت با تصمیم‌گیری چندحالته
    بر اساس SRS محصول سوم
    """
    
    num_scenarios = 400
    decision_data = []
    
    # ====== تعریف حالت‌ها ======
    modes = ['reconnaissance', 'surveillance', 'pursuit', 'loiter', 'return_home', 'engagement']
    mode_descriptions = {
        'reconnaissance': 'شناسایی - جمع‌آوری اطلاعات اولیه',
        'surveillance': 'نظارت - پایش مستمر منطقه',
        'pursuit': 'تعقیب - دنبال کردن هدف متحرک',
        'loiter': 'گشت‌زنی - انتظار در موقعیت تعیین‌شده',
        'return_home': 'بازگشت - برگشت به پایگاه',
        'engagement': 'درگیری - اقدام نهایی با هدف'
    }
    
    print("🚀 Generating Product 3 (Decision System) data...")
    
    for i in range(num_scenarios):
        # ====== متغیرهای ورودی ======
        battery_level = np.random.uniform(10, 100)
        threat_level = np.random.uniform(0, 1)
        comm_strength = np.random.uniform(0.1, 1.0)
        target_visibility = np.random.uniform(0.1, 1.0)
        mission_priority = np.random.choice(['critical', 'high', 'medium', 'low'], p=[0.15, 0.35, 0.30, 0.20])
        distance_to_home = np.random.uniform(0.5, 20)
        
        # ====== شرایط محیطی ======
        weather = np.random.choice(['clear', 'cloudy', 'rain', 'fog'], p=[0.5, 0.25, 0.15, 0.10])
        time_of_day = np.random.choice(['day', 'dusk', 'night'])
        wind_speed = np.random.exponential(5)  # متر بر ثانیه
        
        # ====== محاسبه امتیاز حالت‌ها ======
        scores = {}
        
        # 1. Reconnaissance: شناسایی
        scores['reconnaissance'] = (
            (battery_level / 100) * 0.4 +
            (1 - threat_level) * 0.3 +
            comm_strength * 0.2 +
            (1 - distance_to_home / 20) * 0.1
        )
        
        # 2. Surveillance: نظارت
        scores['surveillance'] = (
            target_visibility * 0.5 +
            (1 - threat_level) * 0.2 +
            (battery_level / 100) * 0.2 +
            (1 - wind_speed / 15) * 0.1
        )
        
        # 3. Pursuit: تعقیب
        scores['pursuit'] = (
            target_visibility * 0.6 +
            (battery_level > 40) * 0.2 +
            (1 - threat_level * 0.5) * 0.1 +
            (comm_strength > 0.5) * 0.1
        )
        
        # 4. Loiter: گشت‌زنی
        scores['loiter'] = (
            (1 - comm_strength) * 0.3 +
            (threat_level > 0.7) * 0.3 +
            (1 - target_visibility) * 0.2 +
            (battery_level / 100) * 0.2
        )
        
        # 5. Return Home: بازگشت
        scores['return_home'] = (
            (1 - battery_level / 100) * 0.4 +
            threat_level * 0.3 +
            (distance_to_home < 3) * 0.2 +
            (1 - comm_strength) * 0.1
        )
        
        # 6. Engagement: درگیری
        scores['engagement'] = (
            target_visibility * 0.5 +
            (threat_level > 0.6) * 0.3 +
            (battery_level > 50) * 0.1 -
            distance_to_home * 0.02 +
            (mission_priority == 'critical') * 0.2
        )
        
        # ====== تنظیم وزن‌ها بر اساس اولویت مأموریت ======
        if mission_priority == 'critical':
            scores['engagement'] *= 1.3
            scores['surveillance'] *= 1.1
        elif mission_priority == 'low':
            scores['loiter'] *= 1.3
            scores['return_home'] *= 1.2
            scores['reconnaissance'] *= 0.8
        
        # ====== تأثیر شرایط محیطی ======
        if weather == 'rain':
            for mode in ['surveillance', 'pursuit']:
                scores[mode] *= 0.7
        elif weather == 'fog':
            for mode in ['reconnaissance', 'surveillance']:
                scores[mode] *= 0.6
        
        if time_of_day == 'night':
            scores['reconnaissance'] *= 0.5
            scores['surveillance'] *= 0.7
        
        # ====== انتخاب بهترین حالت ======
        best_mode = max(scores, key=scores.get)
        best_score = scores[best_mode]
        
        # ====== اطمینان تصمیم‌گیری ======
        # بر اساس فاصله از دومین حالت برتر
        sorted_scores = sorted(scores.values(), reverse=True)
        confidence = 0.5 + 0.5 * (1 - (sorted_scores[1] / sorted_scores[0])) if len(sorted_scores) > 1 else 0.9
        confidence = min(0.99, max(0.3, confidence))
        
        # ====== نویز تصمیم‌گیری (خطای سنسور یا پردازش) ======
        if np.random.random() < 0.07 * (1 - confidence):  # خطا در شرایط نامطمئن
            possible_modes = [m for m in modes if m != best_mode]
            best_mode = np.random.choice(possible_modes)
            best_score = scores[best_mode]
            confidence = confidence * 0.8
        
        # ====== ذخیره رکورد ======
        decision_data.append({
            'scenario_id': i,
            'battery_level': round(battery_level, 1),
            'threat_level': round(threat_level, 3),
            'comm_strength': round(comm_strength, 3),
            'target_visibility': round(target_visibility, 3),
            'mission_priority': mission_priority,
            'distance_to_home_km': round(distance_to_home, 2),
            'weather': weather,
            'time_of_day': time_of_day,
            'wind_speed': round(wind_speed, 1),
            'reconnaissance_score': round(scores['reconnaissance'], 4),
            'surveillance_score': round(scores['surveillance'], 4),
            'pursuit_score': round(scores['pursuit'], 4),
            'loiter_score': round(scores['loiter'], 4),
            'return_home_score': round(scores['return_home'], 4),
            'engagement_score': round(scores['engagement'], 4),
            'selected_mode': best_mode,
            'selected_mode_score': round(best_score, 4),
            'decision_confidence': round(confidence, 3),
            'is_optimal_decision': int(confidence > 0.8),
            'meets_performance_spec': 'YES' if (confidence > 0.7 and best_score > 0.3) else 'NO'
        })
    
    return pd.DataFrame(decision_data)

# ========== تولید و ذخیره داده ==========
print("\n" + "="*60)
print("PRODUCT 3 - MISSION DECISION SYSTEM DATA")
print("="*60)

df_decision = generate_mission_decision_data()
df_decision.to_csv('mission_decision_data.csv', index=False)

# ========== تحلیل داده ==========
print(f"\n📊 Total scenarios: {len(df_decision)}")
print(f"Modes distribution:")
print(df_decision['selected_mode'].value_counts())

print("\n--- Mode Selection by Priority ---")
priority_analysis = df_decision.groupby(['mission_priority', 'selected_mode']).size().unstack(fill_value=0)
print(priority_analysis)

print("\n--- Decision Confidence Statistics ---")
print(f"Average confidence: {df_decision['decision_confidence'].mean():.3f}")
print(f"High confidence (> 0.8): {df_decision[df_decision['decision_confidence'] > 0.8].shape[0]} scenarios")
print(f"Low confidence (< 0.5): {df_decision[df_decision['decision_confidence'] < 0.5].shape[0]} scenarios")

print("\n--- Environmental Impact ---")
weather_impact = df_decision.groupby('weather').agg({
    'decision_confidence': 'mean',
    'selected_mode_score': 'mean'
}).round(3)
print(weather_impact)

print(f"\n✅ Specification compliance: {df_decision['meets_performance_spec'].value_counts(normalize=True)['YES']*100:.1f}%")

print("\n📁 File saved: mission_decision_data.csv")
print("\n✅ Product 3 data generation complete!")
