// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/GridInvSys_DragDropWidget.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Data/GridInvSys_InventoryItemInstance.h"
#include "Data/InvSys_InventoryItemInstance.h"
#include "Data/InvSys_ItemFragment_DragDrop.h"
#include "Widgets/GridInvSys_DragItemWidget.h"
#include "Data/InvSys_InventoryItemInfo.h"

void UGridInvSys_DragDropWidget::SetItemInstance(UInvSys_InventoryItemInstance* NewItemInstance)
{
	ItemInstance = NewItemInstance;
}


UGridInvSys_ContainerGridItemWidget* UGridInvSys_DragDropWidget::GetGridItemWidget() const
{
	//return GridItemWidget;
	return nullptr;
}

EDragDropType UGridInvSys_DragDropWidget::GetDragDropType() const
{
	return DragDropType;
}

void UGridInvSys_DragDropWidget::SetDragDropType(EDragDropType NewDragType)
{
	DragDropType = NewDragType;
}

FReply UGridInvSys_DragDropWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (ItemInstance)
	{
		FEventReply EventReply = UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton);
		return EventReply.NativeReply;
	}
	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UGridInvSys_DragDropWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent,
	UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

	/*if (UGridInvSys_InventoryItemInstance* GridItemInstance = GetItemInstance<UGridInvSys_InventoryItemInstance>())
	{
		auto DragDropFragment = GridItemInstance->FindFragmentByClass<UGridInvSys_ItemFragment_DragDrop>();
		if (DragDropFragment)
		{
			// 创建 Dragging 控件
			UGridInvSys_DragItemWidget* DraggingWidget =
				CreateWidget<UGridInvSys_DragItemWidget>(this, DragDropFragment->DraggingWidgetClass);
			DraggingWidget->SetItemInstance(GridItemInstance);
	
			UDragDropOperation* DragDropOperation = NewObject<UDragDropOperation>();
			DragDropOperation->Payload = DraggingWidget;
			DragDropOperation->DefaultDragVisual = DraggingWidget;
			DragDropOperation->Pivot = DragDropFragment->DragPivot;
			DragDropOperation->Offset = DragDropFragment->DragOffset;

			OutOperation = DragDropOperation;
		}
	}*/
}
