print(EditorExitCode.EEC_MODIFIED)

local result = editor.adv_control(0, CMD_SOMETHING, 0, 0)

local res = editor.ask_user("Enter something")

if res.ok then
    print("User typed:", res.text)
else
    print("User cancelled")
end
