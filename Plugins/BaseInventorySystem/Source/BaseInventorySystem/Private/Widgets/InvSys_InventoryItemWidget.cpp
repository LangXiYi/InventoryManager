// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/InvSys_InventoryItemWidget.h"

#include "BaseInventorySystem.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/InvSys_InventoryComponent.h"
#include "Widgets/InvSys_DraggingItemWidget.h"
#include "Components/InvSys_InventoryControllerComponent.h"
#include "Data/InvSys_ItemFragment_DragDrop.h"
#include "Data/InvSys_InventoryItemInstance.h"
#include "Library/InvSys_InventorySystemLibrary.h"

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
	return FReply::Unhandled();
}

void UInvSys_InventoryItemWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent,
	UDragDropOperation*& OutOperation)
{
	if (ItemInstance == nullptr)
	{
		return;
	}
	auto DragDropFragment = ItemInstance->FindFragmentByClass<UInvSys_ItemFragment_DragDrop>();
	if (DragDropFragment && bIsWaitingServerResponse == false)
	{
		// 获取当前玩家的库存组件
		UInvSys_DraggingItemWidget* DraggingWidget = CreateWidget<UInvSys_DraggingItemWidget>(this, DragDropFragment->DraggingWidgetClass);
		UInvSys_InventoryControllerComponent* PlayerInvComp = UInvSys_InventorySystemLibrary::GetPlayerInventoryComponent(GetWorld());
		UInvSys_InventoryComponent* FromInvComp = ItemInstance->GetInventoryComponent();
		if (DraggingWidget ==nullptr || PlayerInvComp == nullptr || FromInvComp == nullptr)
		{
			return;
		}
		/** 等待服务器响应用户拖拽事件 */
		bIsWaitingServerResponse = true;

		DraggingWidget->ItemInstance = ItemInstance.Get();

		UDragDropOperation* DragDropOperation = NewObject<UDragDropOperation>();
		DragDropOperation->Payload = ItemInstance.Get();
		DragDropOperation->DefaultDragVisual = DraggingWidget;
		DragDropOperation->Pivot = DragDropFragment->DragPivot;
		DragDropOperation->Offset = DragDropFragment->DragOffset;
		OutOperation = DragDropOperation;
		
		PlayerInvComp->Server_TryDragItemInstance(FromInvComp, ItemInstance.Get()); //通知服务器: 玩家正在拖拽物品
		GetWorld()->GetTimerManager().SetTimer(ServerTimeoutHandle, [&]()
		{
			bIsWaitingServerResponse = false;
			// TODO::发送网络环境较差通知？
		}, 5.f, false);
	}
}

void UInvSys_InventoryItemWidget::NativeDestruct()
{
	GetWorld()->GetTimerManager().ClearTimer(ServerTimeoutHandle);
	Super::NativeDestruct();
}
