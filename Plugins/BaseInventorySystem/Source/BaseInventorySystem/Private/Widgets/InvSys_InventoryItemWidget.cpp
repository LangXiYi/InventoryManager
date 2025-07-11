// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/InvSys_InventoryItemWidget.h"

#include "BaseInventorySystem.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/InvSys_InventoryComponent.h"
#include "Data/InvSys_ItemFragment_DragDrop.h"
#include "Data/InvSys_InventoryItemInstance.h"
#include "Interface/InvSys_DraggingItemInterface.h"

void UInvSys_InventoryItemWidget::SetItemInstance(UInvSys_InventoryItemInstance* NewItemInstance)
{
	ItemInstance = NewItemInstance;
}

FReply UInvSys_InventoryItemWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (ItemInstance.Get())
	{
		FEventReply EventReply = UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton);
		return EventReply.NativeReply;
	}
	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UInvSys_InventoryItemWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent,
	UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

	if (ItemInstance.IsValid() == false)
	{
		return;
	}

	// 获取当前玩家的库存组件
	UInvSys_InventoryComponent* PlayerInvComp = GetPlayerInventoryComponent();
	UInvSys_InventoryComponent* FromInvComp = ItemInstance->GetInventoryComponent();

	auto DragDropFragment = ItemInstance->FindFragmentByClass<UInvSys_ItemFragment_DragDrop>();
	if (DragDropFragment && PlayerInvComp && FromInvComp)
	{
		check(DragDropFragment->DraggingWidgetClass)
		// 创建 Dragging 控件
		UUserWidget* DraggingWidget = CreateWidget<UUserWidget>(this, DragDropFragment->DraggingWidgetClass);
		if (DraggingWidget && DraggingWidget->Implements<UInvSys_DraggingItemInterface>())
		{
			PlayerInvComp->SetDraggingWidget(DraggingWidget); //更新库存组件中被拖拽的物品控件，方便更新最新的物品实例。
			IInvSys_DraggingItemInterface::Execute_UpdateItemInstance(DraggingWidget, ItemInstance.Get());


			UDragDropOperation* DragDropOperation = NewObject<UDragDropOperation>();
			DragDropOperation->Payload = this;
			DragDropOperation->DefaultDragVisual = DraggingWidget;
			DragDropOperation->Pivot = DragDropFragment->DragPivot;
			DragDropOperation->Offset = DragDropFragment->DragOffset;
			OutOperation = DragDropOperation;

			PlayerInvComp->Server_TryDragItemInstance(FromInvComp, ItemInstance.Get()); //通知服务器: 玩家正在拖拽物品
		}
	}
}

void UInvSys_InventoryItemWidget::NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	Super::NativeOnDragCancelled(InDragDropEvent, InOperation);
	/*UInvSys_InventoryComponent* PlayerGridInvComp = GetPlayerInventoryComponent();
	if (PlayerGridInvComp)
	{
		PlayerGridInvComp->SetDraggingWidget(nullptr);
		UInvSys_InventoryComponent* FromInvComp = ItemInstance->GetInventoryComponent();
		PlayerGridInvComp->Server_CancelDragItemInstance(FromInvComp); //通知服务器: 玩家正在取消拖拽
	}
	UE_LOG(LogInventorySystem, Error, TEXT("Cancel Drag"))*/
}