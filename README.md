# HXstomp_Midi

![](01_midi.jpg)

## Banks and Functions

| Bank              |            |          0          |            |
| :---------------- | :--------: | :-----------------: | :--------: |
| **Description**   |            |    Snapshot Mode    |            |
| **On Bank Enter** |            | Stomp to Stomp mode |            |
| **Button**        |   **A**    |        **B**        |   **C**    |
| **Short Press**   | Snapshot 1 |     Snapshot 2      | Snapshot 3 |
| **Long Press**    | FS Mode Up |    Boost CC 100     |  Bank Up   |

---

| Bank              |              |         1          |               |
| :---------------- | :----------: | :----------------: | :-----------: |
| **Description**   |              |     Tap/Tuner      |               |
| **On Bank Enter** |              | Stomp to Snap mode |               |
| **Button**        |    **A**     |       **B**        |     **C**     |
| **Short Press**   | Toggle tuner |     Tap Tempo      | Toggle Bypass |
| **Long Press**    |  FS Mode Up  |    Boost CC 100    |    Bank Up    |

---

| Bank              |            |         2          |               |
| :---------------- | :--------: | :----------------: | :-----------: |
| **Description**   |            |      FS Mode       |               |
| **On Bank Enter** |            | Stomp to Snap mode |               |
| **Button**        |   **A**    |       **B**        |     **C**     |
| **Short Press**   |    FS4     |        FS5         | Toggle CC 101 |
| **Long Press**    | FS Mode Up |    Boost CC 100    |    Bank Up    |

---

| Bank              |            |         3          |           |
| :---------------- | :--------: | :----------------: | :-------: |
| **Description**   |            |    Looper Mode     |           |
| **On Bank Enter** |            | Stomp to Snap mode |           |
| **Button**        |   **A**    |       **B**        |   **C**   |
| **Short Press**   | Rec/OverD  |     Play/Stop      | Undo/Redo |
| **Long Press**    | FS Mode Up |    Boost CC 100    |  Bank Up  |

- **Looper mode can be enabled / disabled on startup**
  - By long pressing any button while LED is white
- After the white light the LED flashed 3 times
  - White if looper is disabled
  - Red if looper is enabled
- The looper state (enabled / disabled) is preserved
During Operation:    
- LED flashing **fast** during recording / overdubbing
- LED flashing **slow** during playback

## Routing Layout

![](HX_Stomp_Midi_Steckplatine.png)
![](HX_Stomp_Midi_Schaltplan.png)
