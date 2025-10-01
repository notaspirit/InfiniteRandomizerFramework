# Infinite Randomizer Framework

Infinite Randomizer Framework (IRF) is a modding framework designed to replace older solutions like 4x/8x Poster and Magazine Frameworks. Unlike previous frameworks that relied on removing and re-adding nodes via ArchiveXL, a process that is fragile, inflexible, and prone to conflicts, IRF dynamically replaces resource paths as they load.  

This approach eliminates major issues in older frameworks and provides additional benefits, including true randomization every time resources are loaded. IRF currently supports `.mi` (decals), `.ent`, and `.mesh` resources that are directly loaded by streaming sectors.

---

## Table of Contents

1. [Requirements](#requirements)
2. [Key Benefits](#key-benefits)
3. [Installation](#installation)
4. [Implementation](#implementation)

---

## Requirements

### Mandatory

* **CyberEngineTweaks** ([GitHub](https://github.com/maximegmd/CyberEngineTweaks) | [Nexus](https://www.nexusmods.com/cyberpunk2077/mods/107))
* **RedScript** ([GitHub](https://github.com/jac3km4/redscript) | [Nexus](https://www.nexusmods.com/cyberpunk2077/mods/1511))

---

## Key Benefits

- Mods implementing IRF do not conflict with each other
- Version-independent
- Different randomization on every load
- Weighted distribution for replacements
- Supports modded streaming sectors
- Works with `.mi` (decals), `.ent`, and `.mesh` resources
- Configurable source and replacement pools
- Easy to implement, resources just need to be registered, no forced project structure
- Backwards compatible with mods made for existing frameworks

---

## Installation

Install like any other Cyberpunk 2077 mod, either manually or via a mod manager.

---

## Implementation

Mods relying on this framework require a [Variant Pool](https://wiki.redmodding.org/cyberpunk-2077-modding/modding-guides/world-editing/infinite-randomizer-framework/variant-pool) containing the replacement resources and the [Category](https://wiki.redmodding.org/cyberpunk-2077-modding/modding-guides/world-editing/infinite-randomizer-framework/category) it's targeting. For detailed information follow the links.

---
