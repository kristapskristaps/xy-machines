#include "extension.cpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>

using namespace opxy;
using namespace opxy::extensions;
namespace {
void require(bool value, const char* message) {
    if (!value) { std::fprintf(stderr, "%s\n", message); std::exit(1); }
}
u32 calls, popups;
CheckpointCommand last;
CheckpointResult outcome = CheckpointResult::success;
char message[32];
CheckpointResult perform(const Context&, CheckpointCommand command) { ++calls; last = command; return outcome; }
void notify(const char* text) { ++popups; std::snprintf(message, sizeof message, "%s", text); }
void key(u32 key, bool pressed = true, bool accepted = true, u32 modifier = keys::metronome) {
    const Context context = {};
    const Actions actions(nullptr, notify, nullptr, perform);
    checkpoint::hooks.queued_control(context, Control(key, pressed, accepted, false, 0, modifier), actions);
}
}
int main() {
    key(keys::plus); require(calls == 1 && popups == 1 && last == CheckpointCommand::store, "store gesture");
    require(!std::strcmp(message, "checkpoint stored"), "store confirmation");
    key(keys::minus); require(calls == 2 && last == CheckpointCommand::restore, "restore gesture");
    key(keys::record); require(calls == 3 && last == CheckpointCommand::undo, "recovery gesture");
    key(keys::plus, false); key(keys::plus, true, false); key(keys::plus, true, true, keys::none); key(7);
    require(calls == 3 && popups == 3, "unaccepted controls do nothing");
    const CheckpointResult refusals[] = {CheckpointResult::empty, CheckpointResult::wrong_scene,
        CheckpointResult::busy, CheckpointResult::changed, CheckpointResult::unavailable};
    for (const auto result : refusals) {
        outcome = result; key(keys::minus);
        require(std::strcmp(message, "checkpoint restored") && std::strlen(message) < 32, "visible refusal");
    }
    outcome = CheckpointResult::empty; key(keys::record);
    require(!std::strcmp(message, "nothing to recover"), "empty recovery message");
    return 0;
}
