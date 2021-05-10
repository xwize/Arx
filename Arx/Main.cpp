#include "Arx.h"

#include <cstdio>
#include <vector>

int main() {

    arx::context_ context;
	arx::error_ e;

	FILE* p = nullptr;
	fopen_s(&p, "out.a", "rb");
	if (!p) {
		printf("Failed to open out.a\n");
		return 1;
	}

	context.user = (void*)p;
	context.read = [](void* buf, size_t elementSize, size_t count, void* p) -> size_t {
		return fread(buf, elementSize, count, (FILE*)p);
	};
	context.tell = [](void* p) -> long {
		return ftell((FILE*)p);
	};
	context.seek = [](void* p, long offset, int origin) -> int {
		return fseek((FILE*)p, offset, origin);
	};
	context.write = [](void* buf, size_t elementSize, size_t count, void* p)->size_t {
		return fwrite(buf, elementSize, count, (FILE*)p);
	};

	arx::UseContext(context);

	arx::archiveHeader_ archHead;
	if ((e = arx::ReadArchiveHeader(&archHead)) != arx::error_::kNone) {
		printf("%s\n", arx::to_string(e));
		fclose(p);
		return 1;
	}

	while (!arx::CheckEnd()) {
		arx::memberHeader_ memHead;
		
		if ((e = arx::ReadMemberHeader(&memHead)) != arx::error_::kNone) {
			printf("%s\n", arx::to_string(e));
			fclose(p);
			return 1;
		}

		printf("%s %d %d\n", memHead.ident, memHead.identLen, memHead.size);

		std::vector<unsigned char> bytes;
		bytes.resize(memHead.size);
		arx::ReadMember(memHead, &bytes[0], memHead.size);

		//arx::SkipMember(memHead);
	}
	fclose(p);

    return 0;
}
