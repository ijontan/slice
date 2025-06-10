# 3rd person ARPG Slicing machenic testing game

A machanic testing game built using [raylib](https://www.raylib.com/) and [entt](https://github.com/skypjack/entt). Slicing enemy and physics!!

## 🎮 Features

- 3D environment
- ECS architecture using `entt`
- physics!!!
- detachable enemy body
- enemy Body generation using flood fill
- Custom raymarching shaders

## 🕹️ Controls

| Action         | Key |
|----------------|-----|
| Move           | `W`, `S` |
| Rotate (pitch/yaw/roll) | Arrow keys, `A`, `D` |
| Shoot          | `Space` or `Left Mouse Button` |
| Look Back      | `Shift` |
| Restart        | `R` |

## 📸 Snapshots

### Physics
![Collision](assets/demo/physics_demo_collision.gif)
![2k Body](assets/demo/physics_demo_2k_body.gif)

### Enemy Body Regen
![Body Regen](assets/demo/body_regen.gif)


## Getting Started

### Prerequisites

- C++ compiler (supporting C++17)
- linux or equivalent environment (macos | windows wsl)

### Installation
```bash
git clone
```

### Build and run
```bash
make
./a.out
```
By default `make` is `make run`
