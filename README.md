# Mili-Multi-Mode-Adaptive-Decision-System

**Multi-Mode Adaptive Decision System** — adaptive operational-mode selection for UAV missions (STM32H7 target, 10 Hz, 6 modes).

## Quick Start

```bash
pip install -r requirements.txt
python scripts/generate_mission_data.py --scenarios 400
cmake -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build
ctest --test-dir build -C Release --output-on-failure
```

**Windows:** use `C:\mili-build` as build dir to avoid MAX_PATH — see [docs/INSTALL.md](docs/INSTALL.md).

| Guide | Link |
|-------|------|
| Install & troubleshoot | [docs/INSTALL.md](docs/INSTALL.md) |
| Build (incl. Windows) | [docs/BUILD.md](docs/BUILD.md) |
| All documentation | [docs/README.md](docs/README.md) |

---

Multi-Mode Adaptive Decision System
Multi-Mode Adaptive Decision System
1. Introduction
1.1 Purpose
This document describes the complete specification of an intelligent decision system that automatically selects the best drone operating mode based on environmental and mission conditions.

1.2 Scope
6 operating modes: reconnaissance, surveillance, pursuit, loiter, return, engagement

Decision-making based on 5 components: battery, threat, communication, visibility, priority

Simultaneous optimization of energy, mission success and survival

Update rate: 10 Hz

1.3 Definitions and Acronyms
Acronym	Description
MCDM	Multi-Criteria Decision Making
TOPSIS	Technique for Order of Preference by Similarity to Ideal Solution
FSM	Finite State Machine
QoS	Quality of Service
2. Overall Requirements
2.1 Product Perspective
This system runs as the decision layer on the main processor (STM32H7) and uses sensor data and the outputs of products 1 and 2.

text
┌─────────────────────────────────────────────────┐
│         Adaptive Decision System               │
├─────────────────────────────────────────────────┤
│  Inputs:                                        │
│  • Battery status (Product 1 - power mgmt)      │
│  • Position/navigation (Product 2 - VIO)        │
│  • Environmental sensor data                    │
│  • Operator commands                            │
├─────────────────────────────────────────────────┤
│  Decision core:                                 │
│  • Score calculation for 6 modes                │
│  • Multi-objective optimization                 │
│  • Best mode selection                          │
├─────────────────────────────────────────────────┤
│  Output:                                        │
│  • Selected operating mode                      │
│  • Mode control parameters                      │
│  • Decision reports                             │
└─────────────────────────────────────────────────┘
2.2 Key Features
Multi-criteria evaluation: combining 5 components with dynamic weighting

Multi-objective optimization: balance between energy, success and survival

Adaptability: weights adjusted based on mission priority

Robustness: decision-making under incomplete data

3. System Requirements
3.1 Hardware Requirements
Component	Specification
Processor	STM32H7 (480 MHz)
Memory	512 KB RAM
Inputs	CAN, UART, SPI
3.2 Software Requirements
Implemented in C++ with reconfigurability

Weighting table adjustable at runtime

Logging of all decisions for post-event analysis

4. Functional Specifications
4.1 FR-1: Mode Score Calculation
Description: Calculate the score of each of the 6 modes based on current conditions.

Scoring formula (based on synthetic data):

text
Score(mode) = Σ(w_i * normalized_factor_i)
Acceptance criteria:

Computation time < 5 ms

Mode selection accuracy > 92% in simulation

4.2 FR-2: Optimal Mode Selection
Description: Select the mode with the highest score and apply it to the control system.

Acceptance criteria:

Mode switch time < 50 ms

Oscillation prevention (hysteresis) with a 5-second delay

4.3 FR-3: Reporting and Analysis
Description: Record all decisions for analysis and algorithm improvement.

Acceptance criteria:

Store the last 1000 decisions

Output readable by an analyst

5. Data Generation Code - Product 3
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
    Generates mission scenario data with multi-mode decision-making
    Based on the SRS of product 3
    """
    
    num_scenarios = 400
    decision_data = []
    
    # ====== Mode definitions ======
    modes = ['reconnaissance', 'surveillance', 'pursuit', 'loiter', 'return_home', 'engagement']
    mode_descriptions = {
        'reconnaissance': 'Reconnaissance - initial information gathering',
        'surveillance': 'Surveillance - continuous area monitoring',
        'pursuit': 'Pursuit - tracking a moving target',
        'loiter': 'Loiter - waiting at a designated position',
        'return_home': 'Return - returning to base',
        'engagement': 'Engagement - final action on the target'
    }
    
    print("🚀 Generating Product 3 (Decision System) data...")
    
    for i in range(num_scenarios):
        # ====== Input variables ======
        battery_level = np.random.uniform(10, 100)
        threat_level = np.random.uniform(0, 1)
        comm_strength = np.random.uniform(0.1, 1.0)
        target_visibility = np.random.uniform(0.1, 1.0)
        mission_priority = np.random.choice(['critical', 'high', 'medium', 'low'], p=[0.15, 0.35, 0.30, 0.20])
        distance_to_home = np.random.uniform(0.5, 20)
        
        # ====== Environmental conditions ======
        weather = np.random.choice(['clear', 'cloudy', 'rain', 'fog'], p=[0.5, 0.25, 0.15, 0.10])
        time_of_day = np.random.choice(['day', 'dusk', 'night'])
        wind_speed = np.random.exponential(5)  # meters per second
        
        # ====== Mode score calculation ======
        scores = {}
        
        # 1. Reconnaissance
        scores['reconnaissance'] = (
            (battery_level / 100) * 0.4 +
            (1 - threat_level) * 0.3 +
            comm_strength * 0.2 +
            (1 - distance_to_home / 20) * 0.1
        )
        
        # 2. Surveillance
        scores['surveillance'] = (
            target_visibility * 0.5 +
            (1 - threat_level) * 0.2 +
            (battery_level / 100) * 0.2 +
            (1 - wind_speed / 15) * 0.1
        )
        
        # 3. Pursuit
        scores['pursuit'] = (
            target_visibility * 0.6 +
            (battery_level > 40) * 0.2 +
            (1 - threat_level * 0.5) * 0.1 +
            (comm_strength > 0.5) * 0.1
        )
        
        # 4. Loiter
        scores['loiter'] = (
            (1 - comm_strength) * 0.3 +
            (threat_level > 0.7) * 0.3 +
            (1 - target_visibility) * 0.2 +
            (battery_level / 100) * 0.2
        )
        
        # 5. Return Home
        scores['return_home'] = (
            (1 - battery_level / 100) * 0.4 +
            threat_level * 0.3 +
            (distance_to_home < 3) * 0.2 +
            (1 - comm_strength) * 0.1
        )
        
        # 6. Engagement
        scores['engagement'] = (
            target_visibility * 0.5 +
            (threat_level > 0.6) * 0.3 +
            (battery_level > 50) * 0.1 -
            distance_to_home * 0.02 +
            (mission_priority == 'critical') * 0.2
        )
        
        # ====== Weight adjustment based on mission priority ======
        if mission_priority == 'critical':
            scores['engagement'] *= 1.3
            scores['surveillance'] *= 1.1
        elif mission_priority == 'low':
            scores['loiter'] *= 1.3
            scores['return_home'] *= 1.2
            scores['reconnaissance'] *= 0.8
        
        # ====== Environmental impact ======
        if weather == 'rain':
            for mode in ['surveillance', 'pursuit']:
                scores[mode] *= 0.7
        elif weather == 'fog':
            for mode in ['reconnaissance', 'surveillance']:
                scores[mode] *= 0.6
        
        if time_of_day == 'night':
            scores['reconnaissance'] *= 0.5
            scores['surveillance'] *= 0.7
        
        # ====== Best mode selection ======
        best_mode = max(scores, key=scores.get)
        best_score = scores[best_mode]
        
        # ====== Decision confidence ======
        # Based on the gap to the second-best mode
        sorted_scores = sorted(scores.values(), reverse=True)
        confidence = 0.5 + 0.5 * (1 - (sorted_scores[1] / sorted_scores[0])) if len(sorted_scores) > 1 else 0.9
        confidence = min(0.99, max(0.3, confidence))
        
        # ====== Decision noise (sensor or processing error) ======
        if np.random.random() < 0.07 * (1 - confidence):  # error under uncertain conditions
            possible_modes = [m for m in modes if m != best_mode]
            best_mode = np.random.choice(possible_modes)
            best_score = scores[best_mode]
            confidence = confidence * 0.8
        
        # ====== Save record ======
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

# ========== Generate and save data ==========
print("\n" + "="*60)
print("PRODUCT 3 - MISSION DECISION SYSTEM DATA")
print("="*60)

df_decision = generate_mission_decision_data()
df_decision.to_csv('mission_decision_data.csv', index=False)

# ========== Data analysis ==========
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
