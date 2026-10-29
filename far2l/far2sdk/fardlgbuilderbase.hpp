#pragma once
#ifndef __FAR2SDK_FARDLGBUILDERBASE_H__
#define __FAR2SDK_FARDLGBUILDERBASE_H__

#ifdef UNICODE
# define EMPTY_TEXT L""
#else
# define EMPTY_TEXT ""
#endif

#include "farplug-wide.h"

#include <functional>

// Элемент выпадающего списка в диалоге.
struct DialogBuilderListItem
{
	// Строчка из LNG-файла, которая будет показана в диалоге.
	FarLangMsg MessageId;

	// Значение, которое будет записано в поле Value при выборе этой строчки.
	int ItemValue;
};

struct DialogBuilderListItemWide
{
	// Next to render
	wchar_t* Text;

	// Значение, которое будет записано в поле Value при выборе этой строчки.
	int ItemValue;
};

template<class T>
struct DialogItemBinding
{
	int BeforeLabelID;
	int AfterLabelID;

	int buttonID;
	std::function<void(const int buttonID, void* ctx)> cb;
	void* ctx;

	DialogItemBinding()	: BeforeLabelID(-1), AfterLabelID(-1), buttonID (-1), ctx(nullptr) {}
	DialogItemBinding(int buttonID_, std::function<void(const int buttonID, void* ctx)> _cb, void* _ctx) 
		: BeforeLabelID(-1), AfterLabelID(-1), buttonID(buttonID_), cb(_cb), ctx(_ctx) {}

	virtual ~DialogItemBinding() {}
	virtual void SaveValue(T *Item, int RadioGroupIndex) {}
	virtual void TrySaveValue(T *Item, int RadioGroupIndex) {
		if (buttonID >= 0){ 
			this->SaveValue(Item, RadioGroupIndex);
			cb(buttonID, ctx);
		}
	}

	virtual bool supportsTrySave(){ return buttonID >= 0; }
};

template<class T>
struct CheckBoxBinding: public DialogItemBinding<T>
{
	private:
		BOOL *BValue { nullptr };
		int Mask { 0 };
		bool* bvalue { nullptr };

	public:
		CheckBoxBinding(BOOL *aValue, int aMask) : BValue(aValue), Mask(aMask), bvalue(nullptr) { }
		CheckBoxBinding(bool *aValue, int aMask) : BValue(nullptr), Mask(aMask), bvalue(aValue) { }

		CheckBoxBinding(BOOL *aValue, int aMask, 
			int buttonID_, std::function<void(const int buttonID, void* ctx)> _cb, void* _ctx) 
			: DialogItemBinding<T>(buttonID_, _cb, _ctx), BValue(aValue), Mask(aMask), bvalue(nullptr) { }
		CheckBoxBinding(bool *aValue, int aMask, 
			int buttonID_, std::function<void(const int buttonID, void* ctx)> _cb, void* _ctx) 
			: DialogItemBinding<T>(buttonID_, _cb, _ctx), BValue(nullptr), Mask(aMask), bvalue(aValue) { }

		virtual void SaveValue(T *Item, int RadioGroupIndex)
		{
			if (BValue) {
				if (!Mask) {
					BOOL old = *BValue;
					*BValue = Item->Selected != 0;
					if (old != *BValue)
						fprintf(stderr, "...save checkbox[%p]: selected=%d, BOOL=%d `%ls`\n", Item, Item->Selected, *BValue, Item->strData.CPtr());
				}
				else {
					BOOL old = *BValue;
					if (Item->Selected)
						*BValue |= Mask;
					else
						*BValue &= ~Mask;
					if (old != *BValue)
						fprintf(stderr, "...save checkbox[%p]: selected=%d, mask=%d, BOOL=%d `%ls`\n", Item, Item->Selected, Mask, *BValue, Item->strData.CPtr());
				}
			}
			if (bvalue) {
				bool old = *bvalue;
				*bvalue = Item->Selected != 0;
				if (old != *bvalue)
					fprintf(stderr, "...save checkbox[%p]: selected=%d, bool=%d `%ls`\n", Item, Item->Selected, (int)*bvalue, Item->strData.CPtr());
			}
		}
};

template<class T>
struct CodePageBoxBinding: public DialogItemBinding<T>
{
	private:
		UINT *_Result;
		const UINT *_Chosen;

	public:
		CodePageBoxBinding(UINT *Result, const UINT *Chosen) : _Result(Result), _Chosen(Chosen) { }

		virtual void SaveValue(T *Item, int RadioGroupIndex)
		{
			*_Result = *_Chosen;
		}
};

template<class T>
struct RadioButtonBinding: public DialogItemBinding<T>
{
	private:
		int *Value;

	public:
		RadioButtonBinding(int *aValue) : Value(aValue) { }
		RadioButtonBinding(int *aValue,
			int buttonID_, std::function<void(const int buttonID, void* ctx)> _cb, void* _ctx) 
			: DialogItemBinding<T>(buttonID_, _cb, _ctx), Value(aValue) { }

		virtual void SaveValue(T *Item, int RadioGroupIndex)
		{
			if (Item->Selected)
				*Value = RadioGroupIndex;
		}
};

template<class T>
struct ComboBoxBinding: public DialogItemBinding<T>
{
	int *Value;
	FarList *List;

	ComboBoxBinding(int *aValue, FarList *aList,
		int buttonID_, std::function<void(const int buttonID, void* ctx)> _cb, void* _ctx) 
		: DialogItemBinding<T>(buttonID_, _cb, _ctx), Value(aValue), List(aList) { }
	ComboBoxBinding(int *aValue, FarList *aList) : Value(aValue), List(aList) {	}

	~ComboBoxBinding() 
	{
		delete [] List->Items;
		delete List;
	}

	virtual void SaveValue(T *Item, int RadioGroupIndex)
	{
		FarListItem &ListItem = List->Items[Item->ListPos];
		*Value = ListItem.Reserved[0];
	}
};

template<class T>
struct PushButtonBinding: public DialogItemBinding<T>
{
	PushButtonBinding(int buttonID_, std::function<void(const int buttonID, void* ctx)> _cb, void* _ctx) 
		: DialogItemBinding<T>(buttonID_, _cb, _ctx) {}
	~PushButtonBinding() {}
};

/*
Класс для динамического построения диалогов. Автоматически вычисляет положение и размер
для добавляемых контролов, а также размер самого диалога. Автоматически записывает выбранные
значения в указанное место после закрытия диалога по OK.

По умолчанию каждый контрол размещается в новой строке диалога. Ширина для текстовых строк,
checkbox и radio button вычисляется автоматически, для других элементов передаётся явно.
Есть также возможность добавить статический текст слева или справа от контрола, при помощи
методов AddTextBefore и AddTextAfter.

Поддерживается также возможность расположения контролов в две колонки. Используется следующим
образом:
- StartColumns()
- добавляются контролы для первой колонки
- ColumnBreak()
- добавляются контролы для второй колонки
- EndColumns()

Базовая версия класса используется как внутри кода FAR, так и в плагинах.
*/


template<class T>
class DialogBuilderBase
{
public:
	friend class ItemReference;

	class ItemReference
	{
		DialogBuilderBase<T> &Builder;
		ssize_t Index;

	public:
		ItemReference(DialogBuilderBase<T> &Builder_, ssize_t Index_ = -1)
			: Builder(Builder_), Index(Index_)
		{
		}

		ItemReference &operator =(const ItemReference &src)
		{
			Builder = src.Builder;
			Index = src.Index;
			return *this;
		}

		T *operator ->()
		{
			return (Index >= 0 && Index < Builder.DialogItemsCount) ? &Builder.DialogItems[Index] : nullptr;
		}

		operator T *()
		{
			return (Index >= 0 && Index < Builder.DialogItemsCount) ? &Builder.DialogItems[Index] : nullptr;
		}
	};

	protected:
		T *DialogItems { nullptr };
		DialogItemBinding<T> **Bindings;

		int DialogItemsCount {0};
		int DialogItemsAllocated {0};
		int OKButtonID;
		BOOL UseModernLook {0};
		DWORD dialogFlags {0};

		struct StackedPlace {
			int NextX {5};
			int NextY {2};
			int Column{0};
			int ColumnX{5};
			int ColumnY{2};

			int MaxX{0};
			int MaxY{0};
			int FirstColumnX{5};
			int FirstRowY{2};
		};

		std::vector<StackedPlace> place;
		StackedPlace current {};

		void UpdateNewSizes() {
			current.MaxY = std::max(current.MaxY, current.NextY);
			current.MaxX = std::max(current.MaxX, current.NextX);
		}

		int lastItem() const { return DialogItemsCount - 1; }

	public:
		// New API
		void Add(ItemReference item) {
			item->Y1 = current.NextY;
			item->Y2 = item->Y1 + ItemHeight(item);
			item->X1 = current.NextX;
			current.NextX += item->Width > 0 ? item->Width : ItemWidth(*item);
			item->X2 = current.NextX++;

			UpdateNewSizes();
			current.NextX++; // add gap between elements
		}

		void AddNLBy(ItemReference item) {
			current.NextY += ItemHeight(item) + 1; // for most cases it is zero but for list boxes it will be real size
			UpdateNewSizes();
			current.NextX = current.ColumnX;
		}

		void AddNL() {
			current.NextY ++;
			UpdateNewSizes();
			current.NextX = current.ColumnX;
		}

		void AddColumnView() {
			StackedPlace prev = current;
			place.push_back(prev);

			current = StackedPlace {};

			current.ColumnX = current.FirstColumnX = current.NextX = prev.NextX;
			current.Column = prev.Column + 1;
			current.NextY = current.ColumnY = current.FirstRowY = prev.NextY;
		}

		void AddNewColumn() {
			current.NextY = current.ColumnY;
			current.ColumnX = current.MaxX;
			current.NextX = current.ColumnX;
		}

		void EndColumnView() {
			StackedPlace prev = place.back();
			place.pop_back();

			// int w = current.MaxX - current.FirstColumnX;
			int h = current.MaxY - current.FirstRowY;

			prev.NextY += h;
			prev.MaxY = std::max(current.MaxY, prev.MaxY);
			prev.MaxX = std::max(current.MaxX, prev.MaxX);
			prev.NextX = prev.FirstColumnX;

			current = prev;
		}

		void Indent(int deltax) {
			current.NextX += deltax;
		}

	protected:

		void ReallocDialogItems() {
			DialogItemsAllocated += 32;
			if (!DialogItems) {
				DialogItems = new T[DialogItemsAllocated];
				Bindings = new DialogItemBinding<T> * [DialogItemsAllocated];
			}
			else {
				T *NewDialogItems = new T[DialogItemsAllocated];
				DialogItemBinding<T> **NewBindings = new DialogItemBinding<T> * [DialogItemsAllocated];
				for(int i=0; i<DialogItemsCount; i++) {
					NewDialogItems [i] = DialogItems [i];
					NewBindings [i] = Bindings [i];
				}
				delete [] DialogItems;
				delete [] Bindings;
				DialogItems = NewDialogItems;
				Bindings = NewBindings;
			}
		}

		ItemReference AddDialogItem(int Type, const TCHAR *Text) {
			if (DialogItemsCount == DialogItemsAllocated) ReallocDialogItems();

			int Index = DialogItemsCount++;
			T *Item = &DialogItems [Index];
			InitDialogItem(Item, Text);
			Item->Type = Type;
			Bindings [Index] = nullptr;
			Item->Width = 0;

			return ItemReference(*this, Index);
		}

		int ItemWidth(const T &Item) {
			int Width = 0;

			if (Item.Width > 0)  // width is overridden
				return Item.Width;

			switch(Item.Type) {
			case DI_TEXT:
				return TextWidth(Item);

			case DI_DOUBLEBOX: // text in dialog title
				Width = TextWidth(Item);
				return (Width ? Width + 2 : 0);
			case DI_CHECKBOX:
			case DI_RADIOBUTTON:
			case DI_BUTTON:
				return TextWidth(Item) + 4;
			case DI_EDIT:
			case DI_FIXEDIT:
			case DI_COMBOBOX:
				Width = Item.X2 - Item.X1 + 1;
				/* стрелка history занимает дополнительное место, но раньше она рисовалась поверх рамки
				if (Item.Flags & DIF_HISTORY)
					Width++;
				*/
				return Width;
			}
			return 0;
		}

		int ItemHeight(ItemReference Item) {
			if (Item->Height > 0) return Item->Height;

			switch(Item->Type) {
			case DI_TEXT:
			case DI_DOUBLEBOX:
			case DI_CHECKBOX:
			case DI_RADIOBUTTON:
			case DI_BUTTON:
			case DI_EDIT:
			case DI_FIXEDIT:
			case DI_COMBOBOX:
			default:
				return 0;
			}
			return 0;
		}

        /* border is a special case and it is going first, out of placement rules */
		void AddBorder(const TCHAR *TitleText) {
			T *Title = AddDialogItem(DI_DOUBLEBOX, TitleText);
			Title->X1 = 3;
			Title->Y1 = 1;
		}

		void UpdateBorderSize() {
			T *Title = &DialogItems[0];
			Title->X2 = Title->X1 + MaxTextWidth() + 3;
			Title->Y2 = DialogItems [lastItem()].Y2 + 1;
		}

		int MaxTextWidth() {
			int MaxWidth = ItemWidth(DialogItems [0]); // text in dialog title
			MaxWidth = std::max(MaxWidth, current.MaxX + 3);
			return MaxWidth;
		}

		virtual void InitDialogItem(T *NewDialogItem, const TCHAR *Text) {}
		virtual int TextWidth(const T &Item){ return -1; }

		void SetItemBinding(T* Item, DialogItemBinding<T> *Binding){
			Bindings [GetItemID(Item)] = Binding;
		}

		int GetItemID(T *Item) {
			if (Item) {
				int Index = static_cast<int>(Item - DialogItems);
				if (Index >= 0 && Index < DialogItemsCount)
					return Index;
			}
			return -1;
		}

		DialogItemBinding<T> *FindBinding(T *Item) {
			if (Item) {
				int Index = static_cast<int>(Item - DialogItems);
				if (Index >= 0 && Index < DialogItemsCount)
					return Bindings [Index];
			}
			return nullptr;
		}

		void SaveValues() {
			int RadioGroupIndex = 0;
			for(int i=0; i < DialogItemsCount; i++) {
				if (DialogItems [i].Flags & DIF_GROUP)
					RadioGroupIndex = 0;
				else
					RadioGroupIndex++;

				if (Bindings [i])
					Bindings [i]->SaveValue(&DialogItems [i], RadioGroupIndex);
			}
		}

		virtual const TCHAR *GetLangString(FarLangMsg MessageID) { return nullptr; }
		virtual int DoShowDialog(){ return -1; }

		virtual DialogItemBinding<T> *CreateCheckBoxBinding(BOOL *Value, int Mask){	return nullptr;	}
		virtual DialogItemBinding<T> *CreateCheckBoxBinding(bool *Value, int Mask){	return nullptr;	}
		virtual DialogItemBinding<T> *CreateRadioButtonBinding(int *Value){	return nullptr;	}

		virtual DialogItemBinding<T> *CreateButtonBinding(int ID, std::function<void(const int buttonID, void* ctx)> cb, void* ctx){ return nullptr;	}
		virtual DialogItemBinding<T> *CreateCheckBoxBinding(BOOL *Value, int Mask, int ID, std::function<void(const int buttonID, void* ctx)> cb, void* ctx){	return nullptr;	}
		virtual DialogItemBinding<T> *CreateCheckBoxBinding(bool *Value, int Mask, int ID, std::function<void(const int buttonID, void* ctx)> cb, void* ctx){	return nullptr;	}
		virtual DialogItemBinding<T> *CreateRadioButtonBinding(int *Value, int ID, std::function<void(const int buttonID, void* ctx)> cb, void* ctx){	return nullptr;	}

		DialogBuilderBase(){}

		~DialogBuilderBase() {
			for(int i=0; i < DialogItemsCount; i++) {
				if (Bindings [i])
					delete Bindings [i];
			}
			delete [] DialogItems;
			delete [] Bindings;
		}

	public:
		/* old APi through new one */
		// Добавляет статический текст, расположенный на отдельной строке в диалоге.

		virtual ItemReference AddText(FarLangMsg LabelId, bool newLine = true) {
			auto Item = AddDialogItem(DI_TEXT, GetLangString(LabelId));
			Add(Item);
			if (newLine) AddNL();
			return Item;
		}

		virtual ItemReference AddMultiLineText(FarLangMsg LabelId, int height = 1) {
			auto Item = AddDialogItem(DI_TEXT, GetLangString(LabelId));
			Item->Height = (height > 0 ? height : 1) - 1; /* one-line is added automatically */
			Add(Item);
			AddNLBy(Item);
			return Item;
		}

		// Добавляет чекбокс.
		// BOOL*, bool*, + the same with instant callbacks
		virtual ItemReference AddCheckbox(FarLangMsg TextMessageId, BOOL *Value, bool newLine) {
			return this->AddCheckbox(TextMessageId, Value, 0, newLine);
		}

		virtual ItemReference AddCheckbox(FarLangMsg TextMessageId, BOOL *Value, int Mask = 0, bool newLine = true) {
			auto Item = AddDialogItem(DI_CHECKBOX, GetLangString(TextMessageId));
			if (!Mask)
				Item->Selected = *Value;
			else
				Item->Selected = (*Value & Mask) ;

			Add(Item);
			if (newLine) AddNL();
			SetItemBinding(Item, CreateCheckBoxBinding(Value, Mask));
			return Item;
		}

		virtual ItemReference AddCheckbox(FarLangMsg TextMessageId, bool *Value, bool newLine) {
			return this->AddCheckbox(TextMessageId, Value, 0, newLine);
		}

		virtual ItemReference AddCheckbox(FarLangMsg TextMessageId, bool *Value, int Mask = 0, bool newLine = true) {
			auto Item = AddDialogItem(DI_CHECKBOX, GetLangString(TextMessageId));
			if (!Mask)
				Item->Selected = *Value;
			else
				Item->Selected = (*Value & Mask) ;

			Add(Item);
			if (newLine) AddNL();
			SetItemBinding(Item, CreateCheckBoxBinding(Value, Mask));
			return Item;
		}

		virtual ItemReference AddCheckbox(FarLangMsg TextMessageId, BOOL *Value, bool newLine, 
				std::function<void(const int buttonID, void* ctx)> cb, void* ctx) {
			return this->AddCheckbox(TextMessageId, Value, 0, newLine, cb, ctx);
		}

		virtual ItemReference AddCheckbox(FarLangMsg TextMessageId, BOOL *Value, int Mask, bool newLine, 
				std::function<void(const int buttonID, void* ctx)> cb, void* ctx) 
		{
			auto Item = AddDialogItem(DI_CHECKBOX, GetLangString(TextMessageId));
			if (!Mask)
				Item->Selected = *Value;
			else
				Item->Selected = (*Value & Mask) ;

			Add(Item);
			if (newLine) AddNL();
			SetItemBinding(Item, CreateCheckBoxBinding(Value, Mask, lastItem(), cb, ctx));
			return Item;
		}

		virtual ItemReference AddCheckbox(FarLangMsg TextMessageId, bool *Value, bool newLine, 
				std::function<void(const int buttonID, void* ctx)> cb, void* ctx) {
			return this->AddCheckbox(TextMessageId, Value, 0, newLine, cb, ctx);
		}

		virtual ItemReference AddCheckbox(FarLangMsg TextMessageId, bool *Value, int Mask, bool newLine, 
				std::function<void(const int buttonID, void* ctx)> cb, void* ctx) 
		{
			auto Item = AddDialogItem(DI_CHECKBOX, GetLangString(TextMessageId));
			if (!Mask)
				Item->Selected = *Value;
			else
				Item->Selected = (*Value & Mask) ;

			Add(Item);
			if (newLine) AddNL();
			SetItemBinding(Item, CreateCheckBoxBinding(Value, Mask, lastItem(), cb, ctx));
			return Item;
		}

		// Добавляет группу радиокнопок vertically
		virtual void AddRadioButtons(int *Value, int OptionCount, FarLangMsg MessageIDs[]) {
			for(int i=0; i < OptionCount; i++)
			{
				auto Item = AddDialogItem(DI_RADIOBUTTON, GetLangString(MessageIDs[i]));
				Add(Item);
				AddNL();
				if (!i)
					Item->Flags |= DIF_GROUP;
				if (*Value == i)
					Item->Selected = TRUE;
				SetItemBinding(Item, CreateRadioButtonBinding(Value));
			}
		}

		// Добавляет горизонтальную группу радиокнопок.
		virtual void AddRadioButtonsHorz(int *Value, int OptionCount, FarLangMsg MessageIDs[])	{
			for(int i=0; i < OptionCount; i++) {
				auto Item = AddDialogItem(DI_RADIOBUTTON, GetLangString(MessageIDs[i]));
				Add(Item);
				if (!i) Item->Flags |= DIF_GROUP;
				if (*Value == i) Item->Selected = TRUE;
				SetItemBinding(Item, CreateRadioButtonBinding(Value));
			}
			AddNL();
		}

		// Добавляет группу радиокнопок vertically
		virtual void AddRadioButtons(int *Value, int OptionCount, FarLangMsg MessageIDs[],
				std::function<void(const int buttonID, void* ctx)> cb, void* ctx) 
		{
			for(int i=0; i < OptionCount; i++) {
				auto Item = AddDialogItem(DI_RADIOBUTTON, GetLangString(MessageIDs[i]));
				Add(Item);
				AddNL();
				if (!i)
					Item->Flags |= DIF_GROUP;
				if (*Value == i)
					Item->Selected = TRUE;
				SetItemBinding(Item, CreateRadioButtonBinding(Value, lastItem(), cb, ctx));
			}
		}

		// Добавляет горизонтальную группу радиокнопок.
		virtual void AddRadioButtonsHorz(int *Value, int OptionCount, FarLangMsg MessageIDs[],
				std::function<void(const int buttonID, void* ctx)> cb, void* ctx)	
		{
			for(int i=0; i < OptionCount; i++) {
				auto Item = AddDialogItem(DI_RADIOBUTTON, GetLangString(MessageIDs[i]));
				Add(Item);
				if (!i) Item->Flags |= DIF_GROUP;
				if (*Value == i) Item->Selected = TRUE;
				SetItemBinding(Item, CreateRadioButtonBinding(Value, lastItem(), cb, ctx));
			}
			AddNL();
		}

		virtual ItemReference AddIntEditField(int *Value, int Width, int Flags = 0, bool newLine = true) {
			return ItemReference(*this);
		}

		virtual ItemReference AddDoubleEditField(double *Value, int Width, int Flags = 0, bool newLine = true) {
			return ItemReference(*this);
		}

		ItemReference AddLabeledIntEditField(FarLangMsg LabelId, int *Value, int Width, int Flags = 0, bool newLine = true) {
			AddText(LabelId, false);
			return AddIntEditField(Value, Width, Flags, newLine);
		}

		ItemReference AddIntEditFieldLabeled(FarLangMsg LabelId, int *Value, int Width, int Flags = 0, bool newLine = true) {
			auto edit = AddIntEditField(Value, Width, Flags, false);
			AddText(LabelId, newLine);
			return edit;
		}

		virtual ItemReference AddEditField(TCHAR *Value, int Width, const wchar_t *HistoryID = nullptr, int Flags = 0, bool newLine = true) {
			return ItemReference(*this);
		}

		virtual ItemReference AddEditField(TCHAR *Value, int Width, 
			std::function<void(const int buttonID, void* ctx)> cb, void* ctx,
			const wchar_t *HistoryID = nullptr, int Flags = 0, bool newLine = true)
		{
			return ItemReference(*this);
		}

		virtual ItemReference AddIntEditField(int *Value, int Width, int Flags, bool newLine,
				std::function<void(const int buttonID, void* ctx)> cb, void* ctx) 
		{
			return ItemReference(*this);
		}

		virtual ItemReference AddDoubleEditField(double *Value, int Width, int Flags, bool newLine,
				std::function<void(const int buttonID, void* ctx)> cb, void* ctx) 
		{
			return ItemReference(*this);
		}

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

		// Начинает располагать поля диалога в две колонки.
		void StartColumns()	{ AddColumnView(); }
		void ColumnBreak() { AddNewColumn(); }
		void EndColumns() { EndColumnView(); }

		// Добавляет пустую строку.
		void AddEmptyLine()	{ AddNL(); }

		// Добавляет кнопку
		ItemReference AddButton(FarLangMsg MessageId, int &id, bool newLine = true) {
			auto Button = AddDialogItem(DI_BUTTON, GetLangString(MessageId));
			Add(Button);
			if (newLine) AddNL();
			id = lastItem();
			return Button;
		}

		ItemReference AddButton(FarLangMsg MessageId, int& id, 
				std::function<void(const int buttonID, void* ctx)> cb, void* ctx, bool newLine = true) {
			auto Button = AddDialogItem(DI_BUTTON, GetLangString(MessageId));
			Add(Button);
			if (newLine) AddNL();
			id = lastItem();

			SetItemBinding(Button, CreateButtonBinding(id, cb, ctx));
			Button->Flags |= DIF_BTNNOCLOSE;

			return Button;
		}

		void AddSeparator(FarLangMsg MessageId = FarLangMsg {-1}) {
			if (UseModernLook) AddEmptyLine();

			ItemReference Separator = AddDialogItem(DI_TEXT, MessageId == -1 ? EMPTY_TEXT : GetLangString(MessageId));
			Separator->Flags = DIF_SEPARATOR;
			Add(Separator);
			AddNL();
			Separator->X1 -= 2;

			if (UseModernLook) AddEmptyLine();
		}

		// Добавляет сепаратор, кнопки OK и Cancel.
		void AddOKCancel(FarLangMsg OKMessageId, FarLangMsg CancelMessageId, int* ok = nullptr, int* cancel = nullptr) {
			if (UseModernLook) 
				AddNL();
			else
				AddSeparator();

			auto OKButton = AddDialogItem(DI_BUTTON, GetLangString(OKMessageId));
			OKButton->Flags = DIF_CENTERGROUP;
			OKButton->DefaultButton = TRUE;
			Add(OKButton);
			OKButtonID = lastItem();
			if (ok) *ok = OKButtonID;

			auto CancelButton = AddDialogItem(DI_BUTTON, GetLangString(CancelMessageId));
			CancelButton->Flags = DIF_CENTERGROUP;
			Add(CancelButton);
			if (cancel) *cancel = lastItem();

			AddNL();
		}

		ItemReference AddNone()	{
			return ItemReference(*this);
		}

		void SetDialogFlags(DWORD flags) { dialogFlags = flags; }
		DWORD GetDialogFlags() const { return dialogFlags; };

		bool ShowDialog(int *id = nullptr) {
			UpdateBorderSize();
			int Result = DoShowDialog();
			if (id)	*id = Result;
			if (Result == OKButtonID) {
				SaveValues();
				return true;
			}
			return false;
		}

		void TrySaveValueImmediate(int id) {
			if (Bindings [id] && Bindings [id]->supportsTrySave() )
				Bindings [id]->TrySaveValue(&DialogItems [id], 0);
		}
};
#endif // __FAR2SDK_FARDLGBUILDERBASE_H__
