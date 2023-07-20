// wolf.cc
// 6/9/2015 jichi
#include "engine/model/wolf.h"
#include "engine/enginecontroller.h"
#include "engine/enginedef.h"
#include "engine/enginehash.h"
#include "engine/engineutil.h"
#include "hijack/hijackmanager.h"
#include "memdbg/memsearch.h"
#include "winhook/hookcode.h"
#include "winhook/hookfun.h" 
#include <cstdint>
#include<algorithm>
#define DEBUG "model/wolf"
#include "sakurakit/skdebug.h"
#include<unordered_set>
//#pragma intrinsic(_ReturnAddress)

namespace { // unnamed

namespace ScenarioHook {

namespace Private {

  struct TextListElement // ecx, this structure saved a list of element
  {
    DWORD flag1; // should be zero when text is valid
    LPSTR text;
    DWORD flag2;
    DWORD flag3;
    DWORD flag4;
    int size,
        capacity; // 0xe8, capacity of the data including \0

    bool isScenarioText() const
    { return flag1 == 0 && flag2 == 0 && flag3 == 0 && flag4 == 0; }

    bool isValid() const
    {
      return size > 0 && size <= capacity
        && Engine::isAddressReadable(text, capacity) && size == ::strlen(text);
    }
  };

  // Skip non-printable and special ASCII characters on the left
  inline char *ltrim(char *s)
  {
    while (*s && (uint8_t)*s <= 39)
      s++;
    return s;
  }

  bool hookBefore(winhook::hook_stack *s)
  {
    //enum { DataQueueCapacity = 30 };
    static std::unordered_set<std::string> dataSet_;

    auto self = (TextListElement *)s->ecx; // ecx is actually a list of element
    if (self->isValid()) {
      char *text = ltrim(self->text);
      if (*text) {
        std::string data = text;
        if (dataSet_.find(data)==dataSet_.end()) {
          auto role = text == self->text && self->isScenarioText() ? Engine::ScenarioRole : Engine::OtherRole;
          auto split = s->stack[0]; // retaddr
          auto sig = Engine::hashThreadSignature(role, split);

          enum { SendAllowed = true };
          bool timeout;
          int prefixSize = text - self->text,
              capacity = self->capacity - prefixSize;
          data = EngineController::instance()->dispatchTextASTD(data, role, sig, capacity, SendAllowed, &timeout);
          if (timeout)
            return true;

          dataSet_.insert(data);

          ::memcpy(text, data.c_str(), min(data.size() + 1, capacity));
          self->size = data.size() + prefixSize;
        }
      }
    }
    return true;
  }
   
} // namespace Private

/**
 *  Sample game: DRAGON SLAVE
 *
 *  This function is very long and contains many CharNextA.
 *
 *  0046CCBD   CC               INT3
 *  0046CCBE   CC               INT3
 *  0046CCBF   CC               INT3
 *  0046CCC0   55               PUSH EBP    ; jichi: hook here, text list in ecx
 *  0046CCC1   8BEC             MOV EBP,ESP
 *  0046CCC3   6A FF            PUSH -0x1
 *  0046CCC5   68 62496900      PUSH Game.00694962
 *  0046CCCA   64:A1 00000000   MOV EAX,DWORD PTR FS:[0]
 *  0046CCD0   50               PUSH EAX
 *  0046CCD1   64:8925 00000000 MOV DWORD PTR FS:[0],ESP
 *  0046CCD8   81EC A4030000    SUB ESP,0x3A4
 *  0046CCDE   A1 6CE36C00      MOV EAX,DWORD PTR DS:[0x6CE36C]
 *  0046CCE3   33C5             XOR EAX,EBP
 *  0046CCE5   8945 F0          MOV DWORD PTR SS:[EBP-0x10],EAX
 *  0046CCE8   56               PUSH ESI
 *  0046CCE9   57               PUSH EDI
 *  0046CCEA   898D C4FDFFFF    MOV DWORD PTR SS:[EBP-0x23C],ECX
 *  0046CCF0   68 F9D86900      PUSH Game.0069D8F9
 *  0046CCF5   8B85 C4FDFFFF    MOV EAX,DWORD PTR SS:[EBP-0x23C]
 *  0046CCFB   83C0 1C          ADD EAX,0x1C
 *  0046CCFE   50               PUSH EAX
 *  0046CCFF   E8 4CF10400      CALL Game.004BBE50
 *  0046CD04   83C4 08          ADD ESP,0x8
 *  0046CD07   0FB6C8           MOVZX ECX,AL
 *  0046CD0A   85C9             TEST ECX,ECX
 *  0046CD0C   74 05            JE SHORT Game.0046CD13
 *  0046CD0E   E9 CD460000      JMP Game.004713E0
 *  0046CD13   8B95 C4FDFFFF    MOV EDX,DWORD PTR SS:[EBP-0x23C]
 *  0046CD19   83C2 38          ADD EDX,0x38
 *  0046CD1C   52               PUSH EDX
 *  0046CD1D   8B85 C4FDFFFF    MOV EAX,DWORD PTR SS:[EBP-0x23C]
 *  0046CD23   83C0 1C          ADD EAX,0x1C
 *  0046CD26   50               PUSH EAX
 *  0046CD27   E8 04F30400      CALL Game.004BC030
 *  0046CD2C   83C4 08          ADD ESP,0x8
 *  0046CD2F   0FB6C8           MOVZX ECX,AL
 *  0046CD32   85C9             TEST ECX,ECX
 *  0046CD34   74 0B            JE SHORT Game.0046CD41
 *  0046CD36   8B8D C4FDFFFF    MOV ECX,DWORD PTR SS:[EBP-0x23C]
 *  0046CD3C   E8 4F490000      CALL Game.00471690  ; jichi: hook after here
 *  0046CD41   A1 30456F00      MOV EAX,DWORD PTR DS:[0x6F4530]
 *  0046CD46   99               CDQ
 *  0046CD47   B9 64000000      MOV ECX,0x64
 *  0046CD4C   F7F9             IDIV ECX
 *  0046CD4E   8985 C0FDFFFF    MOV DWORD PTR SS:[EBP-0x240],EAX
 *  0046CD54   DB85 C0FDFFFF    FILD DWORD PTR SS:[EBP-0x240]
 *  0046CD5A   DC4D 0C          FMUL QWORD PTR SS:[EBP+0xC]
 *  0046CD5D   DD5D 0C          FSTP QWORD PTR SS:[EBP+0xC]
 *  0046CD60   A1 30456F00      MOV EAX,DWORD PTR DS:[0x6F4530]
 *  0046CD65   99               CDQ
 *  0046CD66   B9 64000000      MOV ECX,0x64
 *  0046CD6B   F7F9             IDIV ECX
 *  0046CD6D   8985 BCFDFFFF    MOV DWORD PTR SS:[EBP-0x244],EAX
 *  0046CD73   DB85 BCFDFFFF    FILD DWORD PTR SS:[EBP-0x244]
 *  0046CD79   DC4D 14          FMUL QWORD PTR SS:[EBP+0x14]
 *  0046CD7C   DD5D 14          FSTP QWORD PTR SS:[EBP+0x14]
 *  0046CD7F   8B15 C0A86F00    MOV EDX,DWORD PTR DS:[0x6FA8C0]
 *  0046CD85   83E2 01          AND EDX,0x1
 *  0046CD88   75 32            JNZ SHORT Game.0046CDBC
 *  0046CD8A   A1 C0A86F00      MOV EAX,DWORD PTR DS:[0x6FA8C0]
 *  0046CD8F   83C8 01          OR EAX,0x1
 *  0046CD92   A3 C0A86F00      MOV DWORD PTR DS:[0x6FA8C0],EAX
 *  0046CD97   C745 FC 00000000 MOV DWORD PTR SS:[EBP-0x4],0x0
 *  0046CD9E   B9 B0A86F00      MOV ECX,Game.006FA8B0
 *  0046CDA3   E8 78210300      CALL Game.0049EF20
 *  0046CDA8   68 20806900      PUSH Game.00698020
 *  0046CDAD   E8 0B020600      CALL Game.004CCFBD
 *  0046CDB2   83C4 04          ADD ESP,0x4
 *  0046CDB5   C745 FC FFFFFFFF MOV DWORD PTR SS:[EBP-0x4],-0x1
 *  0046CDBC   0FB60D C0E26C00  MOVZX ECX,BYTE PTR DS:[0x6CE2C0]
 *  0046CDC3   85C9             TEST ECX,ECX
 *  0046CDC5   0F84 63010000    JE Game.0046CF2E
 *  0046CDCB   C605 C0E26C00 00 MOV BYTE PTR DS:[0x6CE2C0],0x0
 *  0046CDD2   6A 50            PUSH 0x50
 *  0046CDD4   B9 90436F00      MOV ECX,Game.006F4390
 *  0046CDD9   E8 C2190300      CALL Game.0049E7A0
 *  0046CDDE   6A 50            PUSH 0x50
 *  0046CDE0   B9 B0436F00      MOV ECX,Game.006F43B0
 *  0046CDE5   E8 B6190300      CALL Game.0049E7A0
 *  0046CDEA   6A 50            PUSH 0x50
 *  0046CDEC   B9 A0436F00      MOV ECX,Game.006F43A0
 *  0046CDF1   E8 AA190300      CALL Game.0049E7A0
 *  0046CDF6   6A 50            PUSH 0x50
 *  0046CDF8   B9 C0436F00      MOV ECX,Game.006F43C0
 *  0046CDFD   E8 9E190300      CALL Game.0049E7A0
 *  0046CE02   6A 0C            PUSH 0xC
 *  0046CE04   B9 003B6F00      MOV ECX,Game.006F3B00
 *  0046CE09   E8 F20CFEFF      CALL Game.0044DB00
 *  0046CE0E   50               PUSH EAX
 *  0046CE0F   B9 B0A86F00      MOV ECX,Game.006FA8B0
 *  0046CE14   E8 87190300      CALL Game.0049E7A0
 *  0046CE19   C745 80 00000000 MOV DWORD PTR SS:[EBP-0x80],0x0
 *  0046CE20   EB 09            JMP SHORT Game.0046CE2B
 *  0046CE22   8B55 80          MOV EDX,DWORD PTR SS:[EBP-0x80]
 *  0046CE25   83C2 01          ADD EDX,0x1
 *  0046CE28   8955 80          MOV DWORD PTR SS:[EBP-0x80],EDX
 *  0046CE2B   6A 0C            PUSH 0xC
 *  0046CE2D   B9 003B6F00      MOV ECX,Game.006F3B00
 *  0046CE32   E8 C90CFEFF      CALL Game.0044DB00
 *  0046CE37   3945 80          CMP DWORD PTR SS:[EBP-0x80],EAX
 *  0046CE3A   0F8D EE000000    JGE Game.0046CF2E
 *  0046CE40   6A 00            PUSH 0x0
 *  0046CE42   6A 02            PUSH 0x2
 *  0046CE44   8B45 80          MOV EAX,DWORD PTR SS:[EBP-0x80]
 *  0046CE47   50               PUSH EAX
 *  0046CE48   6A 0C            PUSH 0xC
 *  0046CE4A   B9 003B6F00      MOV ECX,Game.006F3B00
 *  0046CE4F   E8 0CF2FDFF      CALL Game.0044C060
 *  0046CE54   85C0             TEST EAX,EAX
 *  0046CE56   7D 0C            JGE SHORT Game.0046CE64
 *  0046CE58   C785 B8FDFFFF 00>MOV DWORD PTR SS:[EBP-0x248],0x0
 *  0046CE62   EB 1A            JMP SHORT Game.0046CE7E
 *  0046CE64   6A 00            PUSH 0x0
 *  0046CE66   6A 02            PUSH 0x2
 *  0046CE68   8B4D 80          MOV ECX,DWORD PTR SS:[EBP-0x80]
 *  0046CE6B   51               PUSH ECX
 *  0046CE6C   6A 0C            PUSH 0xC
 *  0046CE6E   B9 003B6F00      MOV ECX,Game.006F3B00
 *  0046CE73   E8 E8F1FDFF      CALL Game.0044C060
 *  0046CE78   8985 B8FDFFFF    MOV DWORD PTR SS:[EBP-0x248],EAX
 *  0046CE7E   6A 00            PUSH 0x0
 *  0046CE80   6A 01            PUSH 0x1
 *  0046CE82   8B55 80          MOV EDX,DWORD PTR SS:[EBP-0x80]
 *  0046CE85   52               PUSH EDX
 *  0046CE86   6A 0C            PUSH 0xC
 *  0046CE88   B9 003B6F00      MOV ECX,Game.006F3B00
 *  0046CE8D   E8 CEF1FDFF      CALL Game.0044C060
 *  0046CE92   85C0             TEST EAX,EAX
 *  0046CE94   7D 0C            JGE SHORT Game.0046CEA2
 *  0046CE96   C785 B4FDFFFF 00>MOV DWORD PTR SS:[EBP-0x24C],0x0
 *  0046CEA0   EB 1A            JMP SHORT Game.0046CEBC
 *  0046CEA2   6A 00            PUSH 0x0
 *  0046CEA4   6A 01            PUSH 0x1
 *  0046CEA6   8B45 80          MOV EAX,DWORD PTR SS:[EBP-0x80]
 *  0046CEA9   50               PUSH EAX
 *  0046CEAA   6A 0C            PUSH 0xC
 *  0046CEAC   B9 003B6F00      MOV ECX,Game.006F3B00
 *  0046CEB1   E8 AAF1FDFF      CALL Game.0044C060
 *  0046CEB6   8985 B4FDFFFF    MOV DWORD PTR SS:[EBP-0x24C],EAX
 *  0046CEBC   6A 00            PUSH 0x0
 *  0046CEBE   6A 00            PUSH 0x0
 *  0046CEC0   8B4D 80          MOV ECX,DWORD PTR SS:[EBP-0x80]
 *  0046CEC3   51               PUSH ECX
 *  0046CEC4   6A 0C            PUSH 0xC
 *  0046CEC6   B9 003B6F00      MOV ECX,Game.006F3B00
 *  0046CECB   E8 90F1FDFF      CALL Game.0044C060
 *  0046CED0   85C0             TEST EAX,EAX
 *  0046CED2   7D 0C            JGE SHORT Game.0046CEE0
 *  0046CED4   C785 B0FDFFFF 00>MOV DWORD PTR SS:[EBP-0x250],0x0
 *  0046CEDE   EB 1A            JMP SHORT Game.0046CEFA
 *  0046CEE0   6A 00            PUSH 0x0
 *  0046CEE2   6A 00            PUSH 0x0
 *  0046CEE4   8B55 80          MOV EDX,DWORD PTR SS:[EBP-0x80]
 *  0046CEE7   52               PUSH EDX
 *  0046CEE8   6A 0C            PUSH 0xC
 *  0046CEEA   B9 003B6F00      MOV ECX,Game.006F3B00
 *  0046CEEF   E8 6CF1FDFF      CALL Game.0044C060
 *  0046CEF4   8985 B0FDFFFF    MOV DWORD PTR SS:[EBP-0x250],EAX
 *  0046CEFA   8B85 B8FDFFFF    MOV EAX,DWORD PTR SS:[EBP-0x248]
 *  0046CF00   50               PUSH EAX
 *  0046CF01   8B8D B4FDFFFF    MOV ECX,DWORD PTR SS:[EBP-0x24C]
 *  0046CF07   51               PUSH ECX
 *  0046CF08   8B95 B0FDFFFF    MOV EDX,DWORD PTR SS:[EBP-0x250]
 *  0046CF0E   52               PUSH EDX
 *  0046CF0F   E8 4CE10700      CALL Game.004EB060
 *  0046CF14   83C4 0C          ADD ESP,0xC
 *  0046CF17   8BF0             MOV ESI,EAX
 *  0046CF19   8B45 80          MOV EAX,DWORD PTR SS:[EBP-0x80]
 *  0046CF1C   50               PUSH EAX
 *  0046CF1D   B9 B0A86F00      MOV ECX,Game.006FA8B0
 *  0046CF22   E8 D9180300      CALL Game.0049E800
 *  0046CF27   8930             MOV DWORD PTR DS:[EAX],ESI
 *  0046CF29  ^E9 F4FEFFFF      JMP Game.0046CE22
 *  0046CF2E   C745 84 00000000 MOV DWORD PTR SS:[EBP-0x7C],0x0
 *  0046CF35   8B8D C4FDFFFF    MOV ECX,DWORD PTR SS:[EBP-0x23C]
 *  0046CF3B   C741 68 00000000 MOV DWORD PTR DS:[ECX+0x68],0x0
 *  0046CF42   8B8D C4FDFFFF    MOV ECX,DWORD PTR SS:[EBP-0x23C]
 *  0046CF48   E8 23FE0200      CALL Game.0049CD70
 *  0046CF4D   8945 C4          MOV DWORD PTR SS:[EBP-0x3C],EAX
 *  0046CF50   8D4D 9C          LEA ECX,DWORD PTR SS:[EBP-0x64]
 *  0046CF53   E8 D8FA0200      CALL Game.0049CA30
 *  0046CF58   C745 FC 01000000 MOV DWORD PTR SS:[EBP-0x4],0x1
 *  0046CF5F   8D4D D4          LEA ECX,DWORD PTR SS:[EBP-0x2C]
 *  0046CF62   E8 C9FA0200      CALL Game.0049CA30
 *  0046CF67   C645 FC 02       MOV BYTE PTR SS:[EBP-0x4],0x2
 *  0046CF6B   8B95 C4FDFFFF    MOV EDX,DWORD PTR SS:[EBP-0x23C]
 *  0046CF71   C742 70 00000000 MOV DWORD PTR DS:[EDX+0x70],0x0
 *  0046CF78   8B85 C4FDFFFF    MOV EAX,DWORD PTR SS:[EBP-0x23C]
 *  0046CF7E   C780 DC000000 00>MOV DWORD PTR DS:[EAX+0xDC],0x0
 *  0046CF88   8B8D C4FDFFFF    MOV ECX,DWORD PTR SS:[EBP-0x23C]
 *  0046CF8E   C741 78 00000000 MOV DWORD PTR DS:[ECX+0x78],0x0
 *  0046CF95   8B15 4C546F00    MOV EDX,DWORD PTR DS:[0x6F544C]
 *  0046CF9B   52               PUSH EDX
 *  0046CF9C   8B8D C4FDFFFF    MOV ECX,DWORD PTR SS:[EBP-0x23C]
 *  0046CFA2   E8 F9480000      CALL Game.004718A0
 *  0046CFA7   8B8D C4FDFFFF    MOV ECX,DWORD PTR SS:[EBP-0x23C]
 *  0046CFAD   8941 74          MOV DWORD PTR DS:[ECX+0x74],EAX
 *  0046CFB0   6A FF            PUSH -0x1
 *  0046CFB2   8B95 C4FDFFFF    MOV EDX,DWORD PTR SS:[EBP-0x23C]
 *  0046CFB8   8B42 78          MOV EAX,DWORD PTR DS:[EDX+0x78]
 *  0046CFBB   50               PUSH EAX
 *  0046CFBC   8B8D C4FDFFFF    MOV ECX,DWORD PTR SS:[EBP-0x23C]
 *  0046CFC2   E8 A9460000      CALL Game.00471670
 *  0046CFC7   50               PUSH EAX
 *  0046CFC8   8B8D C4FDFFFF    MOV ECX,DWORD PTR SS:[EBP-0x23C]
 *  0046CFCE   8B51 74          MOV EDX,DWORD PTR DS:[ECX+0x74]
 *  0046CFD1   52               PUSH EDX
 *  0046CFD2   8B85 C4FDFFFF    MOV EAX,DWORD PTR SS:[EBP-0x23C]
 *  0046CFD8   8B88 DC000000    MOV ECX,DWORD PTR DS:[EAX+0xDC]
 *  0046CFDE   51               PUSH ECX
 *  0046CFDF   B9 90436F00      MOV ECX,Game.006F4390
 *  0046CFE4   E8 17440000      CALL Game.00471400
 *  0046CFE9   8B95 C4FDFFFF    MOV EDX,DWORD PTR SS:[EBP-0x23C]
 *  0046CFEF   C742 5C 00000000 MOV DWORD PTR DS:[EDX+0x5C],0x0
 *  0046CFF6   8B85 C4FDFFFF    MOV EAX,DWORD PTR SS:[EBP-0x23C]
 *  0046CFFC   C740 60 00000000 MOV DWORD PTR DS:[EAX+0x60],0x0
 *  0046D003   8B8D C4FDFFFF    MOV ECX,DWORD PTR SS:[EBP-0x23C]
 *  0046D009   C741 64 00000000 MOV DWORD PTR DS:[ECX+0x64],0x0
 *  0046D010   C745 8C 00000000 MOV DWORD PTR SS:[EBP-0x74],0x0
 *  0046D017   C745 C8 00000000 MOV DWORD PTR SS:[EBP-0x38],0x0
 *  0046D01E   8B15 EC446F00    MOV EDX,DWORD PTR DS:[0x6F44EC]
 *  0046D024   8955 CC          MOV DWORD PTR SS:[EBP-0x34],EDX
 *  0046D027   A1 8C576F00      MOV EAX,DWORD PTR DS:[0x6F578C]
 *  0046D02C   0FBE08           MOVSX ECX,BYTE PTR DS:[EAX]
 *  0046D02F   894D 88          MOV DWORD PTR SS:[EBP-0x78],ECX
 *  0046D032   8B95 C4FDFFFF    MOV EDX,DWORD PTR SS:[EBP-0x23C]
 *  0046D038   0FB682 E0000000  MOVZX EAX,BYTE PTR DS:[EDX+0xE0]
 *  0046D03F   85C0             TEST EAX,EAX
 *  0046D041   74 07            JE SHORT Game.0046D04A
 *  0046D043   C745 8C 00000000 MOV DWORD PTR SS:[EBP-0x74],0x0
 *  0046D04A   C745 B8 C0BDF0FF MOV DWORD PTR SS:[EBP-0x48],0xFFF0BDC0
 *  0046D051   8B8D C4FDFFFF    MOV ECX,DWORD PTR SS:[EBP-0x23C]
 *  0046D057   C781 F8000000 00>MOV DWORD PTR DS:[ECX+0xF8],0x0
 *  0046D061   C745 BC 00000000 MOV DWORD PTR SS:[EBP-0x44],0x0
 *  0046D068   C645 9B 00       MOV BYTE PTR SS:[EBP-0x65],0x0
 *  0046D06C   C745 90 00000000 MOV DWORD PTR SS:[EBP-0x70],0x0
 *  0046D073   C745 94 00000000 MOV DWORD PTR SS:[EBP-0x6C],0x0
 *  0046D07A   C745 C0 00000000 MOV DWORD PTR SS:[EBP-0x40],0x0
 *  0046D081   8B15 28E26C00    MOV EDX,DWORD PTR DS:[0x6CE228]
 *  0046D087   D1E2             SHL EDX,1
 *  0046D089   8B85 C4FDFFFF    MOV EAX,DWORD PTR SS:[EBP-0x23C]
 *  0046D08F   8990 00010000    MOV DWORD PTR DS:[EAX+0x100],EDX
 *  0046D095   813D 30456F00 C8>CMP DWORD PTR DS:[0x6F4530],0xC8
 *  0046D09F   75 1D            JNZ SHORT Game.0046D0BE
 *  0046D0A1   8B8D C4FDFFFF    MOV ECX,DWORD PTR SS:[EBP-0x23C]
 *  0046D0A7   8B81 00010000    MOV EAX,DWORD PTR DS:[ECX+0x100]
 *  0046D0AD   99               CDQ
 *  0046D0AE   2BC2             SUB EAX,EDX
 *  0046D0B0   D1F8             SAR EAX,1
 *  0046D0B2   8B95 C4FDFFFF    MOV EDX,DWORD PTR SS:[EBP-0x23C]
 *  0046D0B8   8982 00010000    MOV DWORD PTR DS:[EDX+0x100],EAX
 *  0046D0BE   8B8D C4FDFFFF    MOV ECX,DWORD PTR SS:[EBP-0x23C]
 *  0046D0C4   E8 C7FC0200      CALL Game.0049CD90
 *  0046D0C9   8B8D C4FDFFFF    MOV ECX,DWORD PTR SS:[EBP-0x23C]
 *  0046D0CF   3941 68          CMP DWORD PTR DS:[ECX+0x68],EAX
 *  0046D0D2   0F8D ED420000    JGE Game.004713C5
 *  0046D0D8   8B55 C4          MOV EDX,DWORD PTR SS:[EBP-0x3C]
 *  0046D0DB   8955 D0          MOV DWORD PTR SS:[EBP-0x30],EDX
 *  0046D0DE   8B85 C4FDFFFF    MOV EAX,DWORD PTR SS:[EBP-0x23C]
 *  0046D0E4   8B48 68          MOV ECX,DWORD PTR DS:[EAX+0x68]
 *  0046D0E7   894D 84          MOV DWORD PTR SS:[EBP-0x7C],ECX
 *  0046D0EA   8B55 C4          MOV EDX,DWORD PTR SS:[EBP-0x3C]
 *  0046D0ED   52               PUSH EDX
 *  0046D0EE   FF15 94926900    CALL DWORD PTR DS:[<&USER32.CharNextA>]  ; user32.CharNextA
 *  0046D0F4   8945 90          MOV DWORD PTR SS:[EBP-0x70],EAX
 *  0046D0F7   8B45 90          MOV EAX,DWORD PTR SS:[EBP-0x70]
 *  0046D0FA   2B45 C4          SUB EAX,DWORD PTR SS:[EBP-0x3C]
 *  0046D0FD   8945 94          MOV DWORD PTR SS:[EBP-0x6C],EAX
 *  0046D100   8B8D C4FDFFFF    MOV ECX,DWORD PTR SS:[EBP-0x23C]
 *  0046D106   8B51 68          MOV EDX,DWORD PTR DS:[ECX+0x68]
 *  0046D109   0355 94          ADD EDX,DWORD PTR SS:[EBP-0x6C]
 *  0046D10C   8B85 C4FDFFFF    MOV EAX,DWORD PTR SS:[EBP-0x23C]
 *  0046D112   8950 68          MOV DWORD PTR DS:[EAX+0x68],EDX
 *  0046D115   8B4D D0          MOV ECX,DWORD PTR SS:[EBP-0x30]
 *  0046D118   51               PUSH ECX
 *  0046D119   FF15 94926900    CALL DWORD PTR DS:[<&USER32.CharNextA>]  ; user32.CharNextA
 *  0046D11F   8945 C4          MOV DWORD PTR SS:[EBP-0x3C],EAX
 *  0046D122   0FB655 08        MOVZX EDX,BYTE PTR SS:[EBP+0x8]
 *  0046D126   85D2             TEST EDX,EDX
 *  0046D128   74 51            JE SHORT Game.0046D17B
 *  0046D12A   0FB645 9B        MOVZX EAX,BYTE PTR SS:[EBP-0x65]
 *  0046D12E   85C0             TEST EAX,EAX
 *  0046D130   74 49            JE SHORT Game.0046D17B
 *  0046D132   8B8D C4FDFFFF    MOV ECX,DWORD PTR SS:[EBP-0x23C]
 *  0046D138   DB41 68          FILD DWORD PTR DS:[ECX+0x68]
 *  0046D13B   8B95 C4FDFFFF    MOV EDX,DWORD PTR SS:[EBP-0x23C]
 *  0046D141   DAA2 F8000000    FISUB DWORD PTR DS:[EDX+0xF8]
 *  0046D147   8B85 C4FDFFFF    MOV EAX,DWORD PTR SS:[EBP-0x23C]
 *  0046D14D   DC98 88000000    FCOMP QWORD PTR DS:[EAX+0x88]
 *  0046D153   DFE0             FSTSW AX
 *  0046D155   F6C4 41          TEST AH,0x41
 *  0046D158   75 21            JNZ SHORT Game.0046D17B
 *  0046D15A   8B8D C4FDFFFF    MOV ECX,DWORD PTR SS:[EBP-0x23C]
 *  0046D160   DB41 68          FILD DWORD PTR DS:[ECX+0x68]
 *  0046D163   8B95 C4FDFFFF    MOV EDX,DWORD PTR SS:[EBP-0x23C]
 *  0046D169   DAA2 F8000000    FISUB DWORD PTR DS:[EDX+0xF8]
 *  0046D16F   8B85 C4FDFFFF    MOV EAX,DWORD PTR SS:[EBP-0x23C]
 *  0046D175   DD98 88000000    FSTP QWORD PTR DS:[EAX+0x88]
 *  0046D17B   0FB64D 08        MOVZX ECX,BYTE PTR SS:[EBP+0x8]
 *  0046D17F   85C9             TEST ECX,ECX
 *  0046D181   74 35            JE SHORT Game.0046D1B8
 *  0046D183   0FB655 9B        MOVZX EDX,BYTE PTR SS:[EBP-0x65]
 *  0046D187   85D2             TEST EDX,EDX
 *  0046D189   75 2D            JNZ SHORT Game.0046D1B8
 *  0046D18B   8B85 C4FDFFFF    MOV EAX,DWORD PTR SS:[EBP-0x23C]
 *  0046D191   DD80 88000000    FLD QWORD PTR DS:[EAX+0x88]
 *  0046D197   E8 54FF0500      CALL Game.004CD0F0
 *  0046D19C   8B8D C4FDFFFF    MOV ECX,DWORD PTR SS:[EBP-0x23C]
 *  0046D1A2   0381 F8000000    ADD EAX,DWORD PTR DS:[ECX+0xF8]
 *  0046D1A8   8B95 C4FDFFFF    MOV EDX,DWORD PTR SS:[EBP-0x23C]
 *  0046D1AE   3942 68          CMP DWORD PTR DS:[EDX+0x68],EAX
 *  0046D1B1   7E 05            JLE SHORT Game.0046D1B8
 *  0046D1B3   E9 0D420000      JMP Game.004713C5
 *  0046D1B8   8B85 C4FDFFFF    MOV EAX,DWORD PTR SS:[EBP-0x23C]
 *  0046D1BE   0FB688 E2000000  MOVZX ECX,BYTE PTR DS:[EAX+0xE2]
 *  0046D1C5   85C9             TEST ECX,ECX
 *  0046D1C7   74 1C            JE SHORT Game.0046D1E5
 *  0046D1C9   8B95 C4FDFFFF    MOV EDX,DWORD PTR SS:[EBP-0x23C]
 *  0046D1CF   8B85 C4FDFFFF    MOV EAX,DWORD PTR SS:[EBP-0x23C]
 *  0046D1D5   8B8A EC000000    MOV ECX,DWORD PTR DS:[EDX+0xEC]
 *  0046D1DB   3B48 68          CMP ECX,DWORD PTR DS:[EAX+0x68]
 *  0046D1DE   7D 05            JGE SHORT Game.0046D1E5
 *  0046D1E0   E9 E0410000      JMP Game.004713C5
 *  0046D1E5   8B95 C4FDFFFF    MOV EDX,DWORD PTR SS:[EBP-0x23C]
 *  0046D1EB   83BA E8000000 00 CMP DWORD PTR DS:[EDX+0xE8],0x0
 *  0046D1F2   7E 1F            JLE SHORT Game.0046D213
 *  0046D1F4   8B85 C4FDFFFF    MOV EAX,DWORD PTR SS:[EBP-0x23C]
 *  0046D1FA   8B88 E4000000    MOV ECX,DWORD PTR DS:[EAX+0xE4]
 *  0046D200   83E9 01          SUB ECX,0x1
 *  0046D203   8B95 C4FDFFFF    MOV EDX,DWORD PTR SS:[EBP-0x23C]
 *  0046D209   3B4A 68          CMP ECX,DWORD PTR DS:[EDX+0x68]
 *  0046D20C   7D 05            JGE SHORT Game.0046D213
 *  0046D20E   E9 B2410000      JMP Game.004713C5
 *  0046D213   8B85 C4FDFFFF    MOV EAX,DWORD PTR SS:[EBP-0x23C]
 *  0046D219   8B48 68          MOV ECX,DWORD PTR DS:[EAX+0x68]
 *  0046D21C   2B4D 84          SUB ECX,DWORD PTR SS:[EBP-0x7C]
 *  0046D21F   51               PUSH ECX
 *  0046D220   8B55 84          MOV EDX,DWORD PTR SS:[EBP-0x7C]
 *  0046D223   52               PUSH EDX
 *  0046D224   8D85 84FEFFFF    LEA EAX,DWORD PTR SS:[EBP-0x17C]
 *  0046D22A   50               PUSH EAX
 *  0046D22B   8B8D C4FDFFFF    MOV ECX,DWORD PTR SS:[EBP-0x23C]
 *  0046D231   E8 4AFC0200      CALL Game.0049CE80 ; jichi; text in [arg1 + 0x4]
 *  0046D236   8985 ACFDFFFF    MOV DWORD PTR SS:[EBP-0x254],EAX
 *  0046D23C   8B8D ACFDFFFF    MOV ECX,DWORD PTR SS:[EBP-0x254]
 *  0046D242   898D A8FDFFFF    MOV DWORD PTR SS:[EBP-0x258],ECX
 *  0046D248   C645 FC 03       MOV BYTE PTR SS:[EBP-0x4],0x3
 *  0046D24C   8B95 A8FDFFFF    MOV EDX,DWORD PTR SS:[EBP-0x258]
 *  0046D252   52               PUSH EDX
 *
 *  This is the function being called
 *  0047168D   CC               INT3
 *  0047168E   CC               INT3
 *  0047168F   CC               INT3
 *  00471690   55               PUSH EBP
 *  00471691   8BEC             MOV EBP,ESP
 *  00471693   83EC 3C          SUB ESP,0x3C
 *  00471696   894D EC          MOV DWORD PTR SS:[EBP-0x14],ECX
 *  00471699   8B45 EC          MOV EAX,DWORD PTR SS:[EBP-0x14]
 *  0047169C   83C0 1C          ADD EAX,0x1C
 *  0047169F   50               PUSH EAX
 *  004716A0   8B4D EC          MOV ECX,DWORD PTR SS:[EBP-0x14]
 *  004716A3   83C1 38          ADD ECX,0x38
 *  004716A6   E8 65B40200      CALL Game.0049CB10
 *  004716AB   8B4D EC          MOV ECX,DWORD PTR SS:[EBP-0x14]
 *  004716AE   81C1 9C000000    ADD ECX,0x9C
 *  004716B4   E8 47CF0200      CALL Game.0049E600
 *  004716B9   8B4D EC          MOV ECX,DWORD PTR SS:[EBP-0x14]
 *  004716BC   81C1 AC000000    ADD ECX,0xAC
 *  004716C2   E8 39CF0200      CALL Game.0049E600
 *  004716C7   8B4D EC          MOV ECX,DWORD PTR SS:[EBP-0x14]
 *  004716CA   81C1 BC000000    ADD ECX,0xBC
 *  004716D0   E8 2BCF0200      CALL Game.0049E600
 *  004716D5   8B4D EC          MOV ECX,DWORD PTR SS:[EBP-0x14]
 *  004716D8   C781 F0000000 00>MOV DWORD PTR DS:[ECX+0xF0],0x0
 *  004716E2   8B55 EC          MOV EDX,DWORD PTR SS:[EBP-0x14]
 *  004716E5   C782 F4000000 00>MOV DWORD PTR DS:[EDX+0xF4],0x0
 *  004716EF   8B45 EC          MOV EAX,DWORD PTR SS:[EBP-0x14]
 *  004716F2   0FB688 98000000  MOVZX ECX,BYTE PTR DS:[EAX+0x98]
 *  004716F9   85C9             TEST ECX,ECX
 *  004716FB   75 20            JNZ SHORT Game.0047171D
 *  004716FD   8B55 EC          MOV EDX,DWORD PTR SS:[EBP-0x14]
 *  00471700   DD05 10DD6900    FLD QWORD PTR DS:[0x69DD10]
 *  00471706   DD9A 88000000    FSTP QWORD PTR DS:[EDX+0x88]
 *  0047170C   8B45 EC          MOV EAX,DWORD PTR SS:[EBP-0x14]
 *  0047170F   DD05 10DD6900    FLD QWORD PTR DS:[0x69DD10]
 *  00471715   DD98 90000000    FSTP QWORD PTR DS:[EAX+0x90]
 *  0047171B   EB 0F            JMP SHORT Game.0047172C
 *  0047171D   8B4D EC          MOV ECX,DWORD PTR SS:[EBP-0x14]
 *  00471720   DD05 B8E26900    FLD QWORD PTR DS:[0x69E2B8]
 *  00471726   DD99 88000000    FSTP QWORD PTR DS:[ECX+0x88]
 *  0047172C   8B55 EC          MOV EDX,DWORD PTR SS:[EBP-0x14]
 *  0047172F   83C2 1C          ADD EDX,0x1C
 *  00471732   52               PUSH EDX
 *  00471733   8B4D EC          MOV ECX,DWORD PTR SS:[EBP-0x14]
 *  00471736   E8 D5B30200      CALL Game.0049CB10
 *  0047173B   C745 F8 00000000 MOV DWORD PTR SS:[EBP-0x8],0x0
 *  00471742   C745 F0 00000000 MOV DWORD PTR SS:[EBP-0x10],0x0
 *  00471749   C745 FC 00000000 MOV DWORD PTR SS:[EBP-0x4],0x0
 *  00471750   8B45 EC          MOV EAX,DWORD PTR SS:[EBP-0x14]
 *  00471753   C780 E4000000 00>MOV DWORD PTR DS:[EAX+0xE4],0x0
 *  0047175D   8B4D EC          MOV ECX,DWORD PTR SS:[EBP-0x14]
 *  00471760   C781 E8000000 00>MOV DWORD PTR DS:[ECX+0xE8],0x0
 *  0047176A   8B55 EC          MOV EDX,DWORD PTR SS:[EBP-0x14]
 *  0047176D   C782 EC000000 00>MOV DWORD PTR DS:[EDX+0xEC],0x0
 *  00471777   8B45 EC          MOV EAX,DWORD PTR SS:[EBP-0x14]
 *  0047177A   C780 F8000000 00>MOV DWORD PTR DS:[EAX+0xF8],0x0
 *  00471784   8B4D EC          MOV ECX,DWORD PTR SS:[EBP-0x14]
 *  00471787   C681 E2000000 00 MOV BYTE PTR DS:[ECX+0xE2],0x0
 *  0047178E   8B55 EC          MOV EDX,DWORD PTR SS:[EBP-0x14]
 *  00471791   C682 E3000000 00 MOV BYTE PTR DS:[EDX+0xE3],0x0
 *  00471798   C745 F4 00000000 MOV DWORD PTR SS:[EBP-0xC],0x0
 *  0047179F   6A 00            PUSH 0x0
 *  004717A1   68 B4E26900      PUSH Game.0069E2B4
 *  004717A6   8B4D EC          MOV ECX,DWORD PTR SS:[EBP-0x14]
 *  004717A9   E8 72B60200      CALL Game.0049CE20
 *  004717AE   8945 F4          MOV DWORD PTR SS:[EBP-0xC],EAX
 *  004717B1   8B45 F4          MOV EAX,DWORD PTR SS:[EBP-0xC]
 *  004717B4   3B05 FCD86900    CMP EAX,DWORD PTR DS:[0x69D8FC]
 *  004717BA   0F84 D3000000    JE Game.00471893
 *  004717C0   8B4D EC          MOV ECX,DWORD PTR SS:[EBP-0x14]
 *  004717C3   DD81 80000000    FLD QWORD PTR DS:[ECX+0x80]
 *  004717C9   8B55 F4          MOV EDX,DWORD PTR SS:[EBP-0xC]
 *  004717CC   83C2 01          ADD EDX,0x1
 *  004717CF   8955 E8          MOV DWORD PTR SS:[EBP-0x18],EDX
 *  004717D2   DB45 E8          FILD DWORD PTR SS:[EBP-0x18]
 *  004717D5   DC0D 28716B00    FMUL QWORD PTR DS:[0x6B7128]
 *  004717DB   DA35 24E26C00    FIDIV DWORD PTR DS:[0x6CE224]
 *  004717E1   DED9             FCOMPP
 *  004717E3   DFE0             FSTSW AX
 *  004717E5   F6C4 41          TEST AH,0x41
 *  004717E8   75 0E            JNZ SHORT Game.004717F8
 *  004717EA   8B45 EC          MOV EAX,DWORD PTR SS:[EBP-0x14]
 *  004717ED   DD80 80000000    FLD QWORD PTR DS:[EAX+0x80]
 *  004717F3   DD5D E0          FSTP QWORD PTR SS:[EBP-0x20]
 *  004717F6   EB 1B            JMP SHORT Game.00471813
 *  004717F8   8B4D F4          MOV ECX,DWORD PTR SS:[EBP-0xC]
 *  004717FB   83C1 01          ADD ECX,0x1
 *  004717FE   894D DC          MOV DWORD PTR SS:[EBP-0x24],ECX
 *  00471801   DB45 DC          FILD DWORD PTR SS:[EBP-0x24]
 *  00471804   DC0D 28716B00    FMUL QWORD PTR DS:[0x6B7128]
 *  0047180A   DA35 24E26C00    FIDIV DWORD PTR DS:[0x6CE224]
 *  00471810   DD5D E0          FSTP QWORD PTR SS:[EBP-0x20]
 *  00471813   DD05 58AB6A00    FLD QWORD PTR DS:[0x6AAB58]
 *  00471819   DC5D E0          FCOMP QWORD PTR SS:[EBP-0x20]
 *  0047181C   DFE0             FSTSW AX
 *  0047181E   F6C4 41          TEST AH,0x41
 *  00471821   75 0B            JNZ SHORT Game.0047182E
 *  00471823   DD05 58AB6A00    FLD QWORD PTR DS:[0x6AAB58]
 *  00471829   DD5D D4          FSTP QWORD PTR SS:[EBP-0x2C]
 *  0047182C   EB 59            JMP SHORT Game.00471887
 *  0047182E   8B55 EC          MOV EDX,DWORD PTR SS:[EBP-0x14]
 *  00471831   DD82 80000000    FLD QWORD PTR DS:[EDX+0x80]
 *  00471837   8B45 F4          MOV EAX,DWORD PTR SS:[EBP-0xC]
 *  0047183A   83C0 01          ADD EAX,0x1
 *  0047183D   8945 D0          MOV DWORD PTR SS:[EBP-0x30],EAX
 *  00471840   DB45 D0          FILD DWORD PTR SS:[EBP-0x30]
 *  00471843   DC0D 28716B00    FMUL QWORD PTR DS:[0x6B7128]
 *  00471849   DA35 24E26C00    FIDIV DWORD PTR DS:[0x6CE224]
 *  0047184F   DED9             FCOMPP
 *  00471851   DFE0             FSTSW AX
 *  00471853   F6C4 41          TEST AH,0x41
 *  00471856   75 0E            JNZ SHORT Game.00471866
 *  00471858   8B4D EC          MOV ECX,DWORD PTR SS:[EBP-0x14]
 *  0047185B   DD81 80000000    FLD QWORD PTR DS:[ECX+0x80]
 *  00471861   DD5D C8          FSTP QWORD PTR SS:[EBP-0x38]
 *  00471864   EB 1B            JMP SHORT Game.00471881
 *  00471866   8B55 F4          MOV EDX,DWORD PTR SS:[EBP-0xC]
 *  00471869   83C2 01          ADD EDX,0x1
 *  0047186C   8955 C4          MOV DWORD PTR SS:[EBP-0x3C],EDX
 *  0047186F   DB45 C4          FILD DWORD PTR SS:[EBP-0x3C]
 *  00471872   DC0D 28716B00    FMUL QWORD PTR DS:[0x6B7128]
 *  00471878   DA35 24E26C00    FIDIV DWORD PTR DS:[0x6CE224]
 *  0047187E   DD5D C8          FSTP QWORD PTR SS:[EBP-0x38]
 *  00471881   DD45 C8          FLD QWORD PTR SS:[EBP-0x38]
 *  00471884   DD5D D4          FSTP QWORD PTR SS:[EBP-0x2C]
 *  00471887   8B45 EC          MOV EAX,DWORD PTR SS:[EBP-0x14]
 *  0047188A   DD45 D4          FLD QWORD PTR SS:[EBP-0x2C]
 *  0047188D   DD98 80000000    FSTP QWORD PTR DS:[EAX+0x80]
 *  00471893   8BE5             MOV ESP,EBP
 *  00471895   5D               POP EBP
 *  00471896   C3               RETN
 *  00471897   CC               INT3
 *  00471898   CC               INT3
 *  00471899   CC               INT3
 */
bool attach(ULONG startAddress, ULONG stopAddress) // attach other text
{
  ULONG addr = MemDbg::findCallerAddressAfterInt3((ULONG)::CharNextA, startAddress, stopAddress);
  //addr = MemDbg::findNearCallAddress(addr, startAddress, stopAddress);
  //if (!addr)
  //  return false;
  return addr && winhook::hook_before(addr, Private::hookBefore);
}


//bool attach_utsusemi(ULONG startAddress, ULONG stopAddress) // 空蝉コミュニクション
//{ 
//    char xx[100];
//    sprintf(xx,"address %x %x", startAddress, stopAddress);
//    DOUT(xx); 
//    const BYTE bytes[] = {
//          0x68,0xe6,0x52,0x81,0x00,
//          0xb9,0xbc,0x6a,0xaa,0x00,
//          0xe8,0x6d,0xe2,0x07,0x00
//    };//0x00457B14 found by textractor
//    ULONG range =std::min(stopAddress - startAddress, (ULONG)0x00300000);
//    ULONG addr = MemDbg::findBytes(bytes, sizeof(bytes), startAddress, stopAddress + range); 
//    if (addr == 0)return false;
//    sprintf(xx, "findBytes %x ", addr);
//    DOUT(xx); 
//    addr = MemDbg::findEnclosingAlignedFunction(addr);
//    addr = MemDbg::findEnclosingAlignedFunction(addr-4);//恰好前面4个CC，再往前一点。
//    if (addr == 0)return false; 
//    sprintf(xx, "findBytes %x ", addr);
//    DOUT(xx);
//    return winhook::hook_before(addr, Private::hookBefore_2);
//}
 
} // namespace ScenarioHook

} // unnamed namespace

/** Public class */

bool WolfRPGEngine::attach()
{
  ULONG startAddress, stopAddress;
  if (!Engine::getProcessMemoryRange(&startAddress, &stopAddress))
    return false;
  if ((!ScenarioHook::attach(startAddress, stopAddress)))//&&(!ScenarioHook::attach_utsusemi(startAddress, stopAddress)))
    return false;

  HijackManager::instance()->attachFunction((ULONG)::GetGlyphOutlineA); // for dynamic encoding
  //if (::GetACP() == 932) // Japanese code page cp932
  //  HijackManager::instance()->attachFunction((ULONG)::CharNextA); // for dynamic encoding
  return true;
}

// EOF

/*
bool attach() // attach other text
{
  ULONG startAddress, stopAddress;
  if (!Engine::getProcessMemoryRange(&startAddress, &stopAddress))
    return false;

  const uint8_t bytes[] = {
    0x55,               // 00471690   55               push ebp
    0x8b,0xec,          // 00471691   8bec             mov ebp,esp
    0x83,0xec, 0x3c,    // 00471693   83ec 3c          sub esp,0x3c
    0x89,0x4d, 0xec,    // 00471696   894d ec          mov dword ptr ss:[ebp-0x14],ecx
    0x8b,0x45, 0xec,    // 00471699   8b45 ec          mov eax,dword ptr ss:[ebp-0x14]
    0x83,0xc0, 0x1c,    // 0047169c   83c0 1c          add eax,0x1c
    0x50,               // 0047169f   50               push eax
    0x8b,0x4d, 0xec,    // 004716a0   8b4d ec          mov ecx,dword ptr ss:[ebp-0x14]
    0x83,0xc1, 0x38     // 004716a3   83c1 38          add ecx,0x38
  };
  ULONG addr = MemDbg::findBytes(bytes, sizeof(bytes), startAddress, stopAddress);
  if (!addr)
    return false;
  addr = MemDbg::findNearCallAddress(addr, startAddress, stopAddress);
  if (!addr)
    return false;
  return winhook::hook_before(addr, Private::hookBefore2);
}

  bool hookBefore2(winhook::hook_stack *s)
  {
    static QByteArray newData_;
    enum { role = Engine::ScenarioRole }; //, sig = Engine::ScenarioThreadSignature };
    auto self = (FunctionSelf *)s->ecx;
    auto text = self->text;
    if (self->isValid() && (uint8_t)*text > 127) {
      QByteArray oldData = text;
      if (newData_ != oldData) {
        auto split = s->stack[0]; // retaddr
        auto sig = Engine::hashThreadSignature(role, split);
        newData_ = EngineController::instance()->dispatchTextA(oldData, role, sig);
        if (newData_ != oldData) {
          self_->size = newData_.size();
          if (newData_.size() < oldData.size())
            ::memset(text + newData_.size(), 0, oldData.size() - newData_.size());
          ::strcpy(text, newData_.constData());
        }
      }
    }
    return true;
  }


  LPSTR text_;
  TextListElement *self_;

  bool hookBefore(winhook::hook_stack *s)
  {
    auto self = (TextListElement *)s->ecx; // ecx is actually a list of element
    if (self && self->isValid())
      self_ = self;
    return true;
  }

  bool hookAfter(winhook::hook_stack *)
  {
    enum { role = Engine::ScenarioRole, sig = Engine::ScenarioThreadSignature };
    if (self_ && self_->isValid()) {
      //auto sig = Engine::hashThreadSignature(role, split);
      QByteArray oldData = self_->text,
                 newData = EngineController::instance()->dispatchTextA(oldData, role, sig);
      if (newData != oldData) {
        ::strcpy(self_->text, newData.constData());
        self_->size = newData.size();
        //if (newData.size() < oldData.size())
        //  ::memset(text + newData.size(), 0, oldData.size() - newData.size());
      }
      self_ = nullptr;
    }
    return true;
  }
*/
/**
 *  Sample game: DRAGON SLAVE
 *
 *  004cd3ad   cc               int3
 *  004cd3ae   cc               int3
 *  004cd3af   cc               int3
 *  004cd3b0   8b4424 0c        mov eax,dword ptr ss:[esp+0xc]	; jichi: text in arg1
 *  004cd3b4   53               push ebx
 *  004cd3b5   85c0             test eax,eax
 *  004cd3b7   74 52            je short .004cd40b
 *
 *  It seems the function takes three arguments, and text in arg1
 *  00126708   004A5917  RETURN to .004A5917 from .004CD3B0
 *  0012670C   08A6F978 ; jichi: text here
 *  00126710   00000040
 *  00126714   0000003B
 *  00126718  /0012673C
 *  0012671C  |004A5892  RETURN to .004A5892 from .004A5900
 *  00126720  |08A6F978
 *  00126724  |0000003B
 *  00126728  |0069A6C8  .0069A6C8
 *  0012672C  |001278E8
 *  00126730  |08A6F978
 *  00126734  |77DAFFCF  RETURN to ntdll.77DAFFCF from ntdll.77DAFFF1
 *
 *  aligned function of the first two CharNextA
 *  text in ecx
 *  0046CCBE   CC               INT3
 *  0046CCBF   CC               INT3
 *  0046CCC0   55               PUSH EBP
 *  0046CCC1   8BEC             MOV EBP,ESP
 *  0046CCC3   6A FF            PUSH -0x1
 *  0046CCC5   68 62496900      PUSH .00694962
 *  0046CCCA   64:A1 00000000   MOV EAX,DWORD PTR FS:[0]
 *  0046CCD0   50               PUSH EAX
 *

0047168E   CC               INT3
0047168F   CC               INT3
00471690   55               PUSH EBP
00471691   8BEC             MOV EBP,ESP
00471693   83EC 3C          SUB ESP,0x3C
00471696   894D EC          MOV DWORD PTR SS:[EBP-0x14],ECX
00471699   8B45 EC          MOV EAX,DWORD PTR SS:[EBP-0x14]
0047169C   83C0 1C          ADD EAX,0x1C
0047169F   50               PUSH EAX

0046CD36   8B8D C4FDFFFF    MOV ECX,DWORD PTR SS:[EBP-0x23C]
0046CD3C   E8 4F490000      CALL Game.00471690
0046CD41   A1 30456F00      MOV EAX,DWORD PTR DS:[0x6F4530]

 */
