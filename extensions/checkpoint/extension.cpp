#include <opxy/extension.hpp>

namespace opxy { namespace extensions { namespace checkpoint {
namespace {
void control(const Context& context, const Control& event, const Actions& actions) {
    if (!event.accepted || !event.pressed || event.modifier != keys::metronome) return;
    CheckpointCommand command;
    const char* done;
    if (event.key == keys::plus) {
        command = CheckpointCommand::store;
        done = "checkpoint stored";
    } else if (event.key == keys::minus) {
        command = CheckpointCommand::restore;
        done = "checkpoint restored";
    } else if (event.key == keys::record) {
        command = CheckpointCommand::undo;
        done = "reload undone";
    } else return;
    if (!actions.checkpoint) return;
    const CheckpointResult result = actions.checkpoint(context, command);
    const char* message = done;
    switch (result) {
        case CheckpointResult::success: break;
        case CheckpointResult::empty:
            message = command == CheckpointCommand::undo ? "nothing to recover" : "no checkpoint"; break;
        case CheckpointResult::wrong_scene: message = "checkpoint in another scene"; break;
        case CheckpointResult::busy: message = "finish recording first"; break;
        case CheckpointResult::changed: message = "patterns changed - store again"; break;
        case CheckpointResult::unavailable: message = "checkpoint unavailable"; break;
    }
    actions.notify(message);
}
}
extern constexpr Hooks hooks = Hooks().with_queued_control(control);
}}}
