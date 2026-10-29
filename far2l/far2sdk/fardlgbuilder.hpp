#pragma once
#ifndef __FAR2SDK_FARDLGBUILDER_H__
#define __FAR2SDK_FARDLGBUILDER_H__

typedef FarLangMsgID FarLangMsg;

#include "fardlgbuilderbase.hpp"

class PluginDialogBuilder;

class DialogAPIBinding: public DialogItemBinding<FarDialogItem>
{
protected:
	const PluginStartupInfo &Info;
	HANDLE *DialogHandle;
	int ID;

	DialogAPIBinding(const PluginStartupInfo &aInfo, HANDLE *aHandle, int aID)
		: Info(aInfo), DialogHandle(aHandle), ID(aID) {}

	DialogAPIBinding(const PluginStartupInfo &aInfo, HANDLE *aHandle, int aID, 
		int buttonID_, std::function<void(const int buttonID, void* ctx)> _cb, void* _ctx) 
		: DialogItemBinding(buttonID_, _cb, _ctx), Info(aInfo), DialogHandle(aHandle), ID(aID) {}
};

class PluginCheckBoxBinding: public DialogAPIBinding
{
	BOOL *Value;
	int Mask;
    bool* value;

public:
	PluginCheckBoxBinding(const PluginStartupInfo &aInfo, HANDLE *aHandle, int aID, BOOL *aValue, int aMask)
		: DialogAPIBinding(aInfo, aHandle, aID),
		  Value(aValue), Mask(aMask), value(nullptr) { }
	PluginCheckBoxBinding(const PluginStartupInfo &aInfo, HANDLE *aHandle, int aID, bool *aValue, int aMask)
		: DialogAPIBinding(aInfo, aHandle, aID),
		  Value(nullptr), Mask(aMask), value(aValue) { }

	PluginCheckBoxBinding(const PluginStartupInfo &aInfo, HANDLE *aHandle, int aID, BOOL *aValue, int aMask, 
		int buttonID_, std::function<void(const int buttonID, void* ctx)> _cb, void* _ctx) 
		: DialogAPIBinding(aInfo, aHandle, aID, buttonID_, _cb, _ctx),
		  Value(aValue), Mask(aMask), value(nullptr) { }
	PluginCheckBoxBinding(const PluginStartupInfo &aInfo, HANDLE *aHandle, int aID, bool *aValue, int aMask, 
		int buttonID_, std::function<void(const int buttonID, void* ctx)> _cb, void* _ctx) 
		: DialogAPIBinding(aInfo, aHandle, aID, buttonID_, _cb, _ctx),
		  Value(nullptr), Mask(aMask), value(aValue) { }

	virtual void SaveValue(FarDialogItem *Item, int RadioGroupIndex)
	{
		BOOL Selected = static_cast<BOOL>(Info.SendDlgMessage(*DialogHandle, DM_GETCHECK, ID, 0));
		if (Value) {
			if (!Mask) {
				*Value = Selected;
			}
			else {
				if (Selected)
					*Value |= Mask;
				else
					*Value &= ~Mask;
			}
		}

		if (value)
			*value = Selected > 0;
	}
};

class PluginRadioButtonBinding: public DialogAPIBinding
{
	private:
		int *Value;

	public:
		PluginRadioButtonBinding(const PluginStartupInfo &aInfo, HANDLE *aHandle, int aID, int *aValue, 
			int buttonID_, std::function<void(const int buttonID, void* ctx)> _cb, void* _ctx)
			: DialogAPIBinding(aInfo, aHandle, aID, buttonID_, _cb, _ctx),
			  Value(aValue)	{ }
		PluginRadioButtonBinding(const PluginStartupInfo &aInfo, HANDLE *aHandle, int aID, int *aValue)
			: DialogAPIBinding(aInfo, aHandle, aID),
			  Value(aValue)	{ }

		virtual void SaveValue(FarDialogItem *Item, int RadioGroupIndex)
		{
			if (Info.SendDlgMessage(*DialogHandle, DM_GETCHECK, ID, 0))
				*Value = RadioGroupIndex;
		}
};

#ifdef UNICODE

class PluginEditFieldBinding: public DialogAPIBinding
{
private:
	TCHAR *Value;
	int MaxSize;

public:
	PluginEditFieldBinding(const PluginStartupInfo &aInfo, HANDLE *aHandle, int aID, TCHAR *aValue, int aMaxSize)
		: DialogAPIBinding(aInfo, aHandle, aID), Value(aValue), MaxSize(aMaxSize) { }

	PluginEditFieldBinding(const PluginStartupInfo &aInfo, HANDLE *aHandle, int aID, TCHAR *aValue, int aMaxSize,
		int buttonID_, std::function<void(const int buttonID, void* ctx)> _cb, void* _ctx)
		: DialogAPIBinding(aInfo, aHandle, aID, buttonID_, _cb, _ctx), Value(aValue), MaxSize(aMaxSize)	{ }

	virtual void SaveValue(FarDialogItem *Item, int RadioGroupIndex)
	{
		const TCHAR *DataPtr = (const TCHAR *) Info.SendDlgMessage(*DialogHandle, DM_GETCONSTTEXTPTR, ID, 0);
		wcsncpy(Value, DataPtr, MaxSize);
	}
};

class PluginIntEditFieldBinding: public DialogAPIBinding
{
private:
	int *Value;
	TCHAR Buffer[32];
	TCHAR Mask[32];

public:
	PluginIntEditFieldBinding(const PluginStartupInfo &aInfo, HANDLE *aHandle, int aID, int *aValue, int Width)
		: DialogAPIBinding(aInfo, aHandle, aID),
		  Value(aValue)
	{
		aInfo.FSF->itoa(*aValue, Buffer, 10);
		int MaskWidth = Width < 31 ? Width : 31;
		for(int i=0; i<MaskWidth; i++)
			Mask[i] = '9';
		Mask[MaskWidth] = '\0';
	}

	PluginIntEditFieldBinding(const PluginStartupInfo &aInfo, HANDLE *aHandle, int aID, int *aValue, int Width,
			int buttonID_, std::function<void(const int buttonID, void* ctx)> _cb, void* _ctx)
		: DialogAPIBinding(aInfo, aHandle, aID, buttonID_, _cb, _ctx),
		  Value(aValue)
	{
		aInfo.FSF->itoa(*aValue, Buffer, 10);
		int MaskWidth = Width < 31 ? Width : 31;
		for(int i=0; i<MaskWidth; i++)
			Mask[i] = '9';
		Mask[MaskWidth] = '\0';
	}

	virtual void SaveValue(FarDialogItem *Item, int RadioGroupIndex)
	{
		const TCHAR *DataPtr = (const TCHAR *) Info.SendDlgMessage(*DialogHandle, DM_GETCONSTTEXTPTR, ID, 0);
		*Value = Info.FSF->atoi(DataPtr);
	}

	TCHAR *GetBuffer()
	{
		return Buffer;
	}

	const TCHAR *GetMask()
	{
		return Mask;
	}
};

class PluginDoubleEditFieldBinding: public DialogAPIBinding
{
private:
	double *Value;
	TCHAR Buffer[32];

public:
	PluginDoubleEditFieldBinding(const PluginStartupInfo &aInfo, HANDLE *aHandle, int aID, double *aValue, int Width)
		: DialogAPIBinding(aInfo, aHandle, aID),
		  Value(aValue)
	{
		swprintf(Buffer, 32, L"%g", *aValue);
	}

	PluginDoubleEditFieldBinding(const PluginStartupInfo &aInfo, HANDLE *aHandle, int aID, double *aValue, int Width,
			int buttonID_, std::function<void(const int buttonID, void* ctx)> _cb, void* _ctx)
		: DialogAPIBinding(aInfo, aHandle, aID, buttonID_, _cb, _ctx),
		  Value(aValue)
	{
		swprintf(Buffer, 32, L"%g", *aValue);
	}

	virtual void SaveValue(FarDialogItem *Item, int RadioGroupIndex)
	{
		const TCHAR *DataPtr = (const TCHAR *) Info.SendDlgMessage(*DialogHandle, DM_GETCONSTTEXTPTR, ID, 0);
		wchar_t *endptr = 0;
		*Value = wcstod(DataPtr, &endptr);
	}

	TCHAR *GetBuffer() { return Buffer;	}
};

#else

class PluginEditFieldBinding: public DialogItemBinding<FarDialogItem>
{
private:
	TCHAR *Value;
	int MaxSize;

public:

	PluginEditFieldBinding(TCHAR *aValue, int aMaxSize)
		: Value(aValue), MaxSize(aMaxSize) { }

	PluginEditFieldBinding(TCHAR *aValue, int aMaxSize, int buttonID_, std::function<void(const int buttonID, void* ctx)> _cb, void* _ctx)
		: DialogItemBinding(buttonID_, _cb, _ctx), Value(aValue), MaxSize(aMaxSize)	{ }

	virtual void SaveValue(FarDialogItem *Item, int RadioGroupIndex) {
		strncpy(Value, Item->Data, MaxSize);
	}
};

class PluginIntEditFieldBinding: public DialogItemBinding<FarDialogItem>
{
private:
	const PluginStartupInfo &Info;
	int *Value;
	TCHAR Mask[32];

public:
	PluginIntEditFieldBinding(const PluginStartupInfo &aInfo, int *aValue, int Width)
		: Info(aInfo), Value(aValue)
	{
		int MaskWidth = Width < 31 ? Width : 31;
		for(int i=0; i<MaskWidth; i++)
			Mask[i] = '9';
		Mask[MaskWidth] = '\0';
	}

	PluginIntEditFieldBinding(const PluginStartupInfo &aInfo, int *aValue, int Width,
		int buttonID_, std::function<void(const int buttonID, void* ctx)> _cb, void* _ctx)
		: DialogItemBinding(buttonID_, _cb, _ctx), Info(aInfo), Value(aValue)
	{
		int MaskWidth = Width < 31 ? Width : 31;
		for(int i=0; i<MaskWidth; i++)
			Mask[i] = '9';
		Mask[MaskWidth] = '\0';
	}

	virtual void SaveValue(FarDialogItem *Item, int RadioGroupIndex) {
		*Value = Info.FSF->atoi(Item->Data);
	}

	const TCHAR *GetMask() { return Mask; }
};

class PluginDoubleEditFieldBinding: public DialogItemBinding<FarDialogItem>
{
private:
	const PluginStartupInfo &Info;
	double *Value;

public:
	PluginIntEditFieldBinding(const PluginStartupInfo &aInfo, double *aValue, int Width,
		int buttonID_, std::function<void(const int buttonID, void* ctx)> _cb, void* _ctx)
		: DialogItemBinding(buttonID_, _cb, _ctx), Info(aInfo), Value(aValue)
	{}

	virtual void SaveValue(FarDialogItem *Item, int RadioGroupIndex) {
		*Value = strtod(Item->Data);
	}
};

#endif

/* vk: todo: add comboboxes here */

/*
Версия класса для динамического построения диалогов, используемая в плагинах к FAR.
*/
class PluginDialogBuilder: public DialogBuilderBase<FarDialogItem>
{
	protected:
		const PluginStartupInfo &Info;
		HANDLE DialogHandle;
		const TCHAR *HelpTopic;

		virtual void InitDialogItem(FarDialogItem *Item, const TCHAR *Text)
		{
			memset(Item, 0, sizeof(FarDialogItem));
#ifdef UNICODE
			Item->PtrData = Text;
#else
			lstrcpyn(Item->Data, Text, sizeof(Item->Data)/sizeof(Item->Data[0]));
#endif
		}

		virtual int TextWidth(const FarDialogItem &Item)
		{
#ifdef UNICODE
			return wcslen(Item.PtrData);
#else
			return wcslen(Item.Data);
#endif
		}

		virtual const TCHAR *GetLangString(FarLangMsg MessageID)
		{
			return Info.GetMsg(Info.ModuleNumber, MessageID);
		}

		virtual int DoShowDialog()
		{
			int Width = DialogItems [0].X2+4;
			int Height = DialogItems [0].Y2+2;
#ifdef UNICODE
			DialogHandle = Info.DialogInit(Info.ModuleNumber, -1, -1, Width, Height,
				HelpTopic, DialogItems, DialogItemsCount, 0, 0, nullptr, 0);
			return Info.DialogRun(DialogHandle);
#else
			return Info.Dialog(Info.ModuleNumber, -1, -1, Width, Height,
				HelpTopic, DialogItems, DialogItemsCount);
#endif
		}

		virtual DialogItemBinding<FarDialogItem> *CreateCheckBoxBinding(BOOL *Value, int Mask)
		{
#ifdef UNICODE
			return new PluginCheckBoxBinding(Info, &DialogHandle, lastItem(), Value, Mask);
#else
			return new CheckBoxBinding<FarDialogItem>(Value, Mask);
#endif
		}

		virtual DialogItemBinding<FarDialogItem> *CreateCheckBoxBinding(bool *Value, int Mask)
		{
#ifdef UNICODE
			return new PluginCheckBoxBinding(Info, &DialogHandle, DialogItemsCount-1, Value, Mask);
#else
			return new CheckBoxBinding<FarDialogItem>(Value, Mask);
#endif
		}

		virtual DialogItemBinding<FarDialogItem> *CreateRadioButtonBinding(BOOL *Value)
		{
#ifdef UNICODE
			return new PluginRadioButtonBinding(Info, &DialogHandle, lastItem(), Value);
#else
			return new RadioButtonBinding<FarDialogItem>(Value);
#endif
		}

		virtual DialogItemBinding<FarDialogItem> *CreateButtonBinding(int id, std::function<void(const int buttonID, void* ctx)> cb, void* ctx){
			return new PushButtonBinding<FarDialogItem>(id, cb, ctx);
		}

		virtual DialogItemBinding<FarDialogItem> *CreateCheckBoxBinding(BOOL *Value, int Mask, int ID, std::function<void(const int buttonID, void* ctx)> cb, void* ctx) {
#ifdef UNICODE
			return new PluginCheckBoxBinding(Info, &DialogHandle, lastItem(), Value, Mask, ID, cb, ctx);
#else
			return new CheckBoxBinding<FarDialogItem>(Value, Mask, id, cb, ctx);
#endif
		}

		virtual DialogItemBinding<FarDialogItem> *CreateCheckBoxBinding(bool *Value, int Mask, int ID, std::function<void(const int buttonID, void* ctx)> cb, void* ctx) {
#ifdef UNICODE
			return new PluginCheckBoxBinding(Info, &DialogHandle, lastItem(), Value, Mask, ID, cb, ctx);
#else
			return new CheckBoxBinding<FarDialogItem>(Value, Mask, id, cb, ctx);
#endif
		}

		virtual DialogItemBinding<FarDialogItem> *CreateRadioButtonBinding(int *Value, int ID, std::function<void(const int buttonID, void* ctx)> cb, void* ctx){
#ifdef UNICODE
			return new PluginRadioButtonBinding(Info, &DialogHandle, lastItem(), Value, ID, cb, ctx);
#else
			return new RadioButtonBinding<FarDialogItem>(Value, id, cb, ctx);
#endif
		}

public:
		PluginDialogBuilder(const PluginStartupInfo &aInfo, FarLangMsg TitleMessageID, const TCHAR *aHelpTopic)
			: Info(aInfo), HelpTopic(aHelpTopic)
		{
			AddBorder(GetLangString(TitleMessageID));
		}

		~PluginDialogBuilder()
		{
#ifdef UNICODE
			Info.DialogFree(DialogHandle);
#endif
		}

		virtual ItemReference AddIntEditField(int *Value, int Width, int Flags = 0, bool newLine = true)
		{
			auto Item = AddDialogItem(DI_FIXEDIT, EMPTY_TEXT);
			Item->Flags |= DIF_MASKEDIT;
			Item->Width = Width;
			PluginIntEditFieldBinding *Binding;
			Binding = new PluginIntEditFieldBinding(Info, &DialogHandle, DialogItemsCount-1, Value, Width);
			Item->PtrData = Binding->GetBuffer();


#ifdef _FAR_NO_NAMELESS_UNIONS
			Item->Param.Mask = Binding->GetMask();
#else
			Item->Mask = Binding->GetMask();
#endif
			Add(Item);
			if (newLine) AddNL();
			SetItemBinding(Item, Binding);
			return Item;
		}

		virtual ItemReference AddDoubleEditField(double *Value, int Width, int Flags = 0, bool newLine = true)
		{
			auto Item = AddDialogItem(DI_FIXEDIT, EMPTY_TEXT);
			Item->Flags |= DIF_MASKEDIT;
			Item->Width = Width;
			PluginDoubleEditFieldBinding *Binding;
			Binding = new PluginDoubleEditFieldBinding(Info, &DialogHandle, DialogItemsCount-1, Value, Width);
			Item->PtrData = Binding->GetBuffer();

			Add(Item);
			if (newLine) AddNL();
			SetItemBinding(Item, Binding);
			return Item;
		}

        /* compatibility */
		virtual ItemReference AddEditField(TCHAR *Value, int Unused, int Width, const TCHAR *HistoryID = nullptr, int Flags = 0, bool newLine = true)
		{
			auto Item = AddDialogItem(DI_EDIT, Value);
			Item->Width = Width;

			if (HistoryID)
			{
#ifdef _FAR_NO_NAMELESS_UNIONS
				Item->Param.History = HistoryID;
#else
				Item->History = HistoryID;
#endif
				Item->Flags |= DIF_HISTORY;
			}

			Add(Item);
			if (newLine) AddNL();

#ifdef UNICODE
			SetItemBinding(Item, new PluginEditFieldBinding(Info, &DialogHandle, lastItem(), Value, Width));
#else
			SetItemBinding(Item, new PluginEditFieldBinding(Value, Width));
#endif
			return Item;
		}

		virtual ItemReference AddEditField(TCHAR *Value, int Width, const TCHAR *HistoryID = nullptr, int Flags = 0, bool newLine = true) override
		{
			auto Item = AddDialogItem(DI_EDIT, Value);
			Item->Width = Width;

			if (HistoryID)
			{
#ifdef _FAR_NO_NAMELESS_UNIONS
				Item->Param.History = HistoryID;
#else
				Item->History = HistoryID;
#endif
				Item->Flags |= DIF_HISTORY;
			}

			Add(Item);
			if (newLine) AddNL();

#ifdef UNICODE
			SetItemBinding(Item, new PluginEditFieldBinding(Info, &DialogHandle, lastItem(), Value, Width));
#else
			SetItemBinding(Item, new PluginEditFieldBinding(Value, Width));
#endif
			return Item;
		}

		virtual ItemReference AddEditField(TCHAR *Value, int Width, 
			std::function<void(const int buttonID, void* ctx)> cb, void* ctx,
			const wchar_t *HistoryID = nullptr, int Flags = 0, bool newLine = true)
		{
			auto item = AddEditField(Value, Width, HistoryID, Flags, newLine);
			int id = lastItem();
#ifdef UNICODE
			SetItemBinding(item, new PluginEditFieldBinding(Info, &DialogHandle, id, Value, Width, id, cb, ctx));
#else
			SetItemBinding(item, new PluginEditFieldBinding(Value, Width, id, cb, ctx));
#endif
			return item;
		}

		virtual ItemReference AddIntEditField(int *Value, int Width, int Flags, bool newLine,
				std::function<void(const int buttonID, void* ctx)> cb, void* ctx) 
		{
			auto item = AddIntEditField(Value, Width, Flags, newLine);
			int id = lastItem();
#ifdef UNICODE
			SetItemBinding(item, new PluginIntEditFieldBinding(Info, &DialogHandle, id, Value, Width, id, cb, ctx));
#else
			SetItemBinding(item, new PluginIntEditFieldBinding(Value, Width, id, cb, ctx));
#endif
			return item;
		}

		virtual ItemReference AddDoubleEditField(double *Value, int Width, int Flags, bool newLine,
				std::function<void(const int buttonID, void* ctx)> cb, void* ctx) 
		{
			auto item = AddDoubleEditField(Value, Width, Flags, newLine);
			int id = lastItem();
#ifdef UNICODE
			SetItemBinding(item, new PluginDoubleEditFieldBinding(Info, &DialogHandle, id, Value, Width, id, cb, ctx));
#else
			SetItemBinding(item, new PluginDoubleEditFieldBinding(Value, Width, id, cb, ctx));
#endif
			return item;
		}

        /* vk: todo: add combo boxes

		virtual ItemReference AddComboBox(int *Value, int Width, DialogBuilderListItem *Items, int ItemCount, DWORD Flags = DIF_NONE, bool newLine = true) { 
			return ItemReference(*this); 
		}
		
		virtual ItemReference AddComboBox(int *Value, int Width, DialogBuilderListItemWide *Items, int ItemCount, DWORD Flags = DIF_NONE, bool newLine = true) { 
			return ItemReference(*this); 
		}

		virtual ItemReference AddComboBox(int *Value, int Width, DialogBuilderListItem *Items, int ItemCount, 
				std::function<void(const int buttonID, void* ctx)> cb, void* ctx, DWORD Flags = DIF_NONE, bool newLine = true) { 
			return ItemReference(*this); 
		}
		
		virtual ItemReference AddComboBox(int *Value, int Width, DialogBuilderListItemWide *Items, int ItemCount, 
				std::function<void(const int buttonID, void* ctx)> cb, void* ctx, DWORD Flags = DIF_NONE, bool newLine = true) { 
			return ItemReference(*this); 
		}
		*/
};
#endif // __FAR2SDK_FARDLGBUILDER_H__
