/*
DlgBuilder.cpp

Динамическое построение диалогов
*/
/*
Copyright (c) 2009 Far Group
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the authors may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "headers.hpp"

#include "lang.hpp"
#include "DialogBuilder.hpp"
#include "dialog.hpp"
#include "codepage.hpp"
#include "config.hpp"

struct EditFieldBinding : public DialogItemBinding<DialogItemEx>
{
	TCHAR *TextValue;
	FARString* aStrValue;

	EditFieldBinding(TCHAR *aTextValue)	: TextValue(aTextValue), aStrValue(nullptr)	{}
	EditFieldBinding(TCHAR *aTextValue, int buttonID_, std::function<void(const int buttonID, void* ctx)> _cb, void* _ctx)
		: DialogItemBinding<DialogItemEx>(buttonID_, _cb, _ctx), TextValue(aTextValue), aStrValue(nullptr) {}
	
	EditFieldBinding(FARString *aValue)	: TextValue(nullptr), aStrValue(aValue)	{}
	EditFieldBinding(FARString *aValue, int buttonID_, std::function<void(const int buttonID, void* ctx)> _cb, void* _ctx)
		: DialogItemBinding<DialogItemEx>(buttonID_, _cb, _ctx), TextValue(nullptr), aStrValue(aValue) {}

	virtual void SaveValue(DialogItemEx *Item, int RadioGroupIndex) { 
		if(aStrValue) *aStrValue = Item->strData; 
		if(TextValue) wcscpy(TextValue, Item->strData); 
	}
};

struct EditFieldIntBinding : public DialogItemBinding<DialogItemEx>
{
	int *IntValue;
	TCHAR Mask[32];

	EditFieldIntBinding(int *aIntValue, int Width) : IntValue(aIntValue)
	{
		int MaskWidth = Width < 31 ? Width : 31;
		for (int i = 0; i < MaskWidth; i++)
			Mask[i] = '9';
		Mask[MaskWidth] = '\0';
	}

	EditFieldIntBinding(int *aValue, int Width, int buttonID_, std::function<void(const int buttonID, void* ctx)> _cb, void* _ctx)
		: DialogItemBinding<DialogItemEx>(buttonID_, _cb, _ctx), IntValue(aValue) 
	{
		int MaskWidth = Width < 31 ? Width : 31;
		for (int i = 0; i < MaskWidth; i++)
			Mask[i] = '9';
		Mask[MaskWidth] = '\0';
	}

	virtual void SaveValue(DialogItemEx *Item, int RadioGroupIndex)
	{
		wchar_t *endptr;
		*IntValue = wcstoul(Item->strData, &endptr, 10);
	}

	const TCHAR *GetMask() { return Mask; }
};

struct EditFieldDoubleBinding : public DialogItemBinding<DialogItemEx>
{
	double *IntValue;

	EditFieldDoubleBinding(double *aIntValue, int Width) :	IntValue(aIntValue)	{}
	EditFieldDoubleBinding(double *aValue, int Width, int buttonID_, std::function<void(const int buttonID, void* ctx)> _cb, void* _ctx)
		: DialogItemBinding<DialogItemEx>(buttonID_, _cb, _ctx), IntValue(aValue) 
	{}

	virtual void SaveValue(DialogItemEx *Item, int RadioGroupIndex)
	{
		wchar_t *endptr;
		*IntValue = wcstod(Item->strData, &endptr);
	}
};

/*
static bool IsEditField(DialogItemEx *Item)
{
	return Item->Type == DI_EDIT || Item->Type == DI_FIXEDIT || Item->Type == DI_PSWEDIT;
}
*/

DialogBuilder::DialogBuilder(FarLangMsg TitleMessageId, const wchar_t *HelpTopic)
	:
	HelpTopic(HelpTopic)
{
	UserDlgProc = nullptr;
	UseModernLook = Opt.Backend.UseModernLook;
	AddBorder(GetLangString(TitleMessageId));
}

DialogBuilder::~DialogBuilder() {}

void DialogBuilder::InitDialogItem(DialogItemEx *Item, const TCHAR *Text)
{
	Item->Clear();
	Item->ID = DialogItemsCount - 1;
	Item->strData = Text;
	Item->Width = 0;
}

int DialogBuilder::TextWidth(const DialogItemEx &Item)
{
	return static_cast<int>(Item.strData.GetLength());
}

const TCHAR *DialogBuilder::GetLangString(FarLangMsg MessageID)
{
	return MessageID;
}

DialogItemBinding<DialogItemEx> *DialogBuilder::CreateCheckBoxBinding(BOOL *Value, int Mask)
{
	return new CheckBoxBinding<DialogItemEx>(Value, Mask);
}

DialogItemBinding<DialogItemEx> *DialogBuilder::CreateCheckBoxBinding(bool *Value, int Mask)
{
	return new CheckBoxBinding<DialogItemEx>(Value, Mask);
}

DialogItemBinding<DialogItemEx> *DialogBuilder::CreateRadioButtonBinding(int *Value)
{
	return new RadioButtonBinding<DialogItemEx>(Value);
}

DialogItemBinding<DialogItemEx> *DialogBuilder::CreateButtonBinding(int ID, std::function<void(const int buttonID, void* ctx)> cb, void* ctx)
{
	return new PushButtonBinding<DialogItemEx>(ID, cb, ctx);	
}

DialogItemBinding<DialogItemEx> *CreateCheckBoxBinding(BOOL *Value, int Mask, int ID, std::function<void(const int buttonID, void* ctx)> cb, void* ctx){
	return new CheckBoxBinding<DialogItemEx>(Value, Mask, ID, cb, ctx);
}

DialogItemBinding<DialogItemEx> *CreateCheckBoxBinding(bool *Value, int Mask, int ID, std::function<void(const int buttonID, void* ctx)> cb, void* ctx){
	return new CheckBoxBinding<DialogItemEx>(Value, Mask, ID, cb, ctx);
}

DialogItemBinding<DialogItemEx> *CreateRadioButtonBinding(int *Value, int ID, std::function<void(const int buttonID, void* ctx)> cb, void* ctx){
	return new RadioButtonBinding<DialogItemEx>(Value, ID, cb, ctx);
}

DialogBuilderBase<DialogItemEx>::ItemReference DialogBuilder::AddEditField(FARString *Value, int Width, const wchar_t *HistoryID, int Flags, bool newLine)
{
	auto Item = AddDialogItem(DI_EDIT, *Value);
	Item->Width = Width;

	Add(Item);
	if (newLine) AddNL();

	if (HistoryID) {
		Item->strHistory = HistoryID;
		Item->Flags|= DIF_HISTORY;
	}
	Item->Flags|= Flags;

	SetItemBinding(Item, new EditFieldBinding(Value));
	return Item;
}

DialogBuilderBase<DialogItemEx>::ItemReference DialogBuilder::AddEditField(TCHAR *Value, int Width, const wchar_t *HistoryID, int Flags, bool newLine)
{
	auto Item = AddDialogItem(DI_EDIT, Value);
	Item->Width = Width;

	Add(Item);
	if (newLine) AddNL();

	if (HistoryID) {
		Item->strHistory = HistoryID;
		Item->Flags|= DIF_HISTORY;
	}
	Item->Flags|= Flags;

	SetItemBinding(Item, new EditFieldBinding(Value));
	return Item;
}

DialogBuilderBase<DialogItemEx>::ItemReference DialogBuilder::AddIntEditField(int *Value, int Width, int Flags, bool newLine)
{
	auto Item = AddDialogItem(DI_FIXEDIT, L"");
	FormatString ValueText;
	ValueText << *Value;
	Item->strData = ValueText;
	Item->Width = Width;

	Add(Item);
	if (newLine) AddNL();

	EditFieldIntBinding *Binding = new EditFieldIntBinding(Value, Width);
	SetItemBinding(Item, Binding);
	Item->Flags|= Flags;
	Item->Flags|= DIF_MASKEDIT;
	Item->strMask = Binding->GetMask();
	return Item;
}

DialogBuilderBase<DialogItemEx>::ItemReference DialogBuilder::AddDoubleEditField(double *Value, int Width, int Flags, bool newLine)
{
	auto Item = AddDialogItem(DI_FIXEDIT, L"");
	wchar_t buf[32];
	swprintf(buf, 32, L"%g", *Value);
	Item->strData = buf;
	Item->Width = Width;

	Add(Item);
	if (newLine) AddNL();

	EditFieldDoubleBinding *Binding = new EditFieldDoubleBinding(Value, Width);
	SetItemBinding(Item, Binding);
	Item->Flags|= Flags;
	Item->Flags|= DIF_MASKEDIT;
	return Item;
}

DialogBuilderBase<DialogItemEx>::ItemReference DialogBuilder::AddComboBox(int *Value, int Width, DialogBuilderListItem *Items, int ItemCount, DWORD Flags, bool newLine)
{
	auto Item = AddDialogItem(DI_COMBOBOX, L"");
	Item->Width = Width;

	Add(Item);
	if (newLine) AddNL();
	Item->Flags|= Flags;

	FarListItem *ListItems = new FarListItem[ItemCount];
	for (int i = 0; i < ItemCount; i++) {
		ListItems[i].Text = Items[i].MessageId;
		ListItems[i].Flags = (*Value == Items[i].ItemValue) ? LIF_SELECTED : 0;
		ListItems[i].Reserved[0] = Items[i].ItemValue;
	}
	FarList *List = new FarList;
	List->Items = ListItems;
	List->ItemsNumber = ItemCount;
	Item->ListItems = List;

	SetItemBinding(Item, new ComboBoxBinding<DialogItemEx>(Value, List));
	return Item;
}

DialogBuilderBase<DialogItemEx>::ItemReference DialogBuilder::AddComboBox(int *Value, int Width, DialogBuilderListItemWide *Items, int ItemCount, DWORD Flags, bool newLine)
{
	auto Item = AddDialogItem(DI_COMBOBOX, L"");
	Item->Width = Width;

	Add(Item);
	if (newLine) AddNL();
	Item->Flags|= Flags;

	FarListItem *ListItems = new FarListItem[ItemCount];
	for (int i = 0; i < ItemCount; i++) {
		ListItems[i].Text = Items[i].Text;
		ListItems[i].Flags = (*Value == Items[i].ItemValue) ? LIF_SELECTED : 0;
		ListItems[i].Reserved[0] = Items[i].ItemValue;
	}
	FarList *List = new FarList;
	List->Items = ListItems;
	List->ItemsNumber = ItemCount;
	Item->ListItems = List;

	SetItemBinding(Item, new ComboBoxBinding<DialogItemEx>(Value, List));
	return Item;
}

// same as before but with callbacks
DialogBuilderBase<DialogItemEx>::ItemReference DialogBuilder::AddEditField(FARString *Value, int Width,
	std::function<void(const int buttonID, void* ctx)> cb, void* ctx,
	const wchar_t *HistoryID, int Flags, bool newLine)
{
	auto Item = AddDialogItem(DI_EDIT, *Value);
	Item->Width = Width;

	Add(Item);
	if (newLine) AddNL();
	Item->Flags|= Flags;
	if (HistoryID) {
		Item->strHistory = HistoryID;
		Item->Flags|= DIF_HISTORY;
	}

	SetItemBinding(Item, new EditFieldBinding(Value, lastItem(), cb, ctx));         \
	return Item;
}

DialogBuilderBase<DialogItemEx>::ItemReference DialogBuilder::AddEditField(TCHAR *Value, int Width,
	std::function<void(const int buttonID, void* ctx)> cb, void* ctx,
	const wchar_t *HistoryID, int Flags, bool newLine)
{
	auto Item = AddDialogItem(DI_EDIT, Value);
	Item->Width = Width;

	Add(Item);
	if (newLine) AddNL();
	Item->Flags|= Flags;
	if (HistoryID) {
		Item->strHistory = HistoryID;
		Item->Flags|= DIF_HISTORY;
	}

	SetItemBinding(Item, new EditFieldBinding(Value, lastItem(), cb, ctx));         \
	return Item;
}

DialogBuilderBase<DialogItemEx>::ItemReference DialogBuilder::AddIntEditField(int *Value, int Width, int Flags, bool newLine,
	std::function<void(const int buttonID, void* ctx)> cb, void* ctx) 
{
	auto Item = AddDialogItem(DI_FIXEDIT, L"");
	FormatString ValueText;
	ValueText << *Value;
	Item->strData = ValueText;
	Item->Width = Width;

	Add(Item);
	if (newLine) AddNL();

	EditFieldIntBinding *Binding = new EditFieldIntBinding(Value, Width, lastItem(), cb, ctx);
	SetItemBinding(Item, Binding);
	Item->Flags|= Flags;
	Item->Flags|= DIF_MASKEDIT;
	Item->strMask = Binding->GetMask();
	return Item;
}

DialogBuilderBase<DialogItemEx>::ItemReference DialogBuilder::AddDoubleEditField(double *Value, int Width, int Flags, bool newLine,
	std::function<void(const int buttonID, void* ctx)> cb, void* ctx)
{
	auto Item = AddDialogItem(DI_FIXEDIT, L"");
	wchar_t buf[32];
	swprintf(buf, 32, L"%g", *Value);
	Item->strData = buf;
	Item->Width = Width;

	Add(Item);
	if (newLine) AddNL();

	EditFieldDoubleBinding *Binding = new EditFieldDoubleBinding(Value, Width, lastItem(), cb, ctx);
	SetItemBinding(Item, Binding);
	Item->Flags|= Flags;
	Item->Flags|= DIF_MASKEDIT;
	return Item;
}

DialogBuilderBase<DialogItemEx>::ItemReference DialogBuilder::AddComboBox(int *Value, int Width, DialogBuilderListItem *Items, int ItemCount, 
		std::function<void(const int buttonID, void* ctx)> cb, void* ctx, DWORD Flags, bool newLine)
{
	auto Item = AddDialogItem(DI_COMBOBOX, L"");
	Item->Width = Width;

	Add(Item);
	if (newLine) AddNL();
	Item->Flags|= Flags;

	FarListItem *ListItems = new FarListItem[ItemCount];
	for (int i = 0; i < ItemCount; i++) {
		ListItems[i].Text = Items[i].MessageId;
		ListItems[i].Flags = (*Value == Items[i].ItemValue) ? LIF_SELECTED : 0;
		ListItems[i].Reserved[0] = Items[i].ItemValue;
	}
	FarList *List = new FarList;
	List->Items = ListItems;
	List->ItemsNumber = ItemCount;
	Item->ListItems = List;

	SetItemBinding(Item, new ComboBoxBinding<DialogItemEx>(Value, List, lastItem(), cb, ctx));
	return Item;
}
		
DialogBuilderBase<DialogItemEx>::ItemReference DialogBuilder::AddComboBox(int *Value, int Width, DialogBuilderListItemWide *Items, int ItemCount, 
		std::function<void(const int buttonID, void* ctx)> cb, void* ctx, DWORD Flags, bool newLine)
{
	auto Item = AddDialogItem(DI_COMBOBOX, L"");
	Item->Width = Width;

	Add(Item);
	if (newLine) AddNL();
	Item->Flags|= Flags;

	FarListItem *ListItems = new FarListItem[ItemCount];
	for (int i = 0; i < ItemCount; i++) {
		ListItems[i].Text = Items[i].Text;
		ListItems[i].Flags = (*Value == Items[i].ItemValue) ? LIF_SELECTED : 0;
		ListItems[i].Reserved[0] = Items[i].ItemValue;
	}
	FarList *List = new FarList;
	List->Items = ListItems;
	List->ItemsNumber = ItemCount;
	Item->ListItems = List;

	SetItemBinding(Item, new ComboBoxBinding<DialogItemEx>(Value, List, lastItem(), cb, ctx));
	return Item;
}

// Legacy: specific code pages combo box

DialogBuilderBase<DialogItemEx>::ItemReference DialogBuilder::AddCodePagesBox(UINT *Value, int Width, bool allowAuto, bool allowAll, bool newLine)
{
	CodePageBoxes.emplace_back(CodePageBox{DialogItemsCount, *Value, allowAuto, allowAll});
	auto Item = AddDialogItem(DI_COMBOBOX, L"");
	Item->Width = Width;

	Add(Item);
	if (newLine) AddNL();
	Item->Flags|= DIF_DROPDOWNLIST | DIF_LISTWRAPMODE | DIF_LISTAUTOHIGHLIGHT;
	SetItemBinding(Item, new CodePageBoxBinding<DialogItemEx>(Value, &CodePageBoxes.back().Value));
	return Item;
}

void DialogBuilder::LinkFlags(DialogItemEx *Parent, DialogItemEx *Target, FarDialogItemFlags Flags,
		bool LinkLabels, bool bParentChecked)
{
	Parent->Flags|= DIF_AUTOMATION;
	if (bParentChecked) {
		Parent->AddAutomation(Target->ID, Flags, DIF_NONE, DIF_NONE, Flags, DIF_NONE, DIF_NONE);
		if (!Parent->Selected)
			Target->Flags|= Flags;
	}
	else {
		Parent->AddAutomation(Target->ID, DIF_NONE, Flags, Flags, DIF_NONE, DIF_NONE, DIF_NONE);
		if (Parent->Selected)
			Target->Flags|= Flags;
	}

	if (LinkLabels) {
		DialogItemBinding<DialogItemEx> *Binding = FindBinding(Target);
		if (Binding) {
			LinkFlagsByID(Parent, Binding->BeforeLabelID, Flags, bParentChecked);
			LinkFlagsByID(Parent, Binding->AfterLabelID, Flags, bParentChecked);
		}
	}
}

void DialogBuilder::LinkFlagsByID(DialogItemEx *Parent, int TargetID, FarDialogItemFlags Flags, bool bParentChecked)
{
	if (TargetID >= 0) {
		if (bParentChecked) {
			Parent->AddAutomation(TargetID, Flags, DIF_NONE, DIF_NONE, Flags, DIF_NONE, DIF_NONE);
			if (!Parent->Selected)
				DialogItems[TargetID].Flags|= Flags;
		}
		else {
			Parent->AddAutomation(TargetID, DIF_NONE, Flags, Flags, DIF_NONE, DIF_NONE, DIF_NONE);
			if (Parent->Selected)
				DialogItems[TargetID].Flags|= Flags;
		}
	}
}

LONG_PTR WINAPI DialogBuilder::DlgProc(HANDLE hDlg, int Msg, int Param1, LONG_PTR Param2)
{
#if 0
	DialogBuilder *Builder = (DialogBuilder *)((Dialog *)hDlg)->GetDialogData();

	if (Builder->UserDlgProc) {
		const LONG_PTR rv = Builder->UserDlgProc(hDlg, Msg, Param1, Param2);
		if (rv != -1)
			return rv;
	}
#endif

	if (Msg == DN_INITDIALOG) {
		DialogBuilder *Builder = (DialogBuilder *)((Dialog *)hDlg)->GetDialogData();
		for (const auto &CB : Builder->CodePageBoxes) {
			FillCodePagesList(hDlg, CB.Index, CB.Value, CB.allowAuto, CB.allowAll);
		}

	} else if (Msg == DN_EDITCHANGE || Msg == DN_LISTCHANGE) {
		DialogBuilder *Builder = (DialogBuilder *)((Dialog *)hDlg)->GetDialogData();
		if (Msg == DN_EDITCHANGE) {
			for (auto &CB : Builder->CodePageBoxes)
				if (Param1 == CB.Index) {
					FarListPos pos{};
					SendDlgMessage(hDlg, DM_LISTGETCURPOS, CB.Index, (LONG_PTR)&pos);
					CB.Value = (UINT)SendDlgMessage(hDlg, DM_LISTGETDATA, CB.Index, pos.SelectPos);
				}
		}
		Builder->TrySaveValueImmediate(Param1);
	} 
	else if (Msg == DN_BTNCLICK) {
		DialogBuilder *Builder = (DialogBuilder *)((Dialog *)hDlg)->GetDialogData();
		if(Builder) Builder->TrySaveValueImmediate(Param1);
	}

	return DefDlgProc(hDlg, Msg, Param1, Param2);
}

bool DialogBuilder::SetUserDlgProc(FARWINDOWPROC UserDlgProc, LONG_PTR UserParam2)
{
	this->UserDlgProc = UserDlgProc;
	this->UserData = UserParam2;

	return true;
}

int DialogBuilder::DoShowDialog()
{
	Dialog Dlg(DialogItems, DialogItemsCount, DlgProc, (LONG_PTR)this);
	Dlg.SetHelp(HelpTopic);
	Dlg.SetPosition(-1, -1, DialogItems[0].X2 + 4, DialogItems[0].Y2 + 2);
	Dlg.SetResizable(true);
	if(GetDialogFlags()) Dlg.SetDialogMode(GetDialogFlags());
	Dlg.Process();
	return Dlg.GetExitCode();
}
