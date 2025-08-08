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
#include "Widgets/Components/InvSys_DragDropOperation.h"

void UInvSys_InventoryItemWidget::SetItemInstance(UInvSys_InventoryItemInstance* NewItemInstance)
{
	ItemInstance = NewItemInstance;
}

FReply UInvSys_InventoryItemWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	FReply bIsHandled = Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
	if (bIsHandled.IsEventHandled()) return bIsHandled; // 如果蓝图实现了处理，则不会继续执行 C++ 定义的逻辑

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
	if (bIsEnableDragItem == false) return;

	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);
	if (OutOperation != nullptr) return; // 如果蓝图实现了处理，则不会继续执行 C++ 定义的逻辑

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

		UDragDropOperation* DragDropOperation = NewObject<UDragDropOperation>(GetOwningPlayer(), DragDropFragment->DragDropOperationClass);
		DragDropOperation->Payload = ItemInstance.Get();
		DragDropOperation->DefaultDragVisual = DraggingWidget;
		DragDropOperation->Pivot = DragDropFragment->DragPivot;
		DragDropOperation->Offset = DragDropFragment->DragOffset;
		OutOperation = DragDropOperation;
		
		PlayerInvComp->Server_DragItemInstance(FromInvComp, ItemInstance.Get()); //通知服务器: 玩家正在拖拽物品
		// GetWorld()->GetTimerManager().SetTimer(ServerTimeoutHandle, [&]()
		// {
		// 	bIsWaitingServerResponse = false;
		// 	// TODO::发送网络环境较差通知？
		// }, 5.f, false);
	}
}

void UInvSys_InventoryItemWidget::NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	Super::NativeOnDragCancelled(InDragDropEvent, InOperation);
	UInvSys_InventoryControllerComponent* PlayerInvComp = UInvSys_InventorySystemLibrary::GetPlayerInventoryComponent(GetWorld());
	if (PlayerInvComp)
	{
		UE_LOG(LogInventorySystem, Warning, TEXT("Cancel Drag"))
		bIsWaitingServerResponse = false;
		PlayerInvComp->Server_CancelDragItemInstance(ItemInstance.Get());
	}
}

void UInvSys_InventoryItemWidget::NativeConstruct()
{
	Super::NativeConstruct();

	auto OnDraggingItemFunc = [this](FGameplayTag Tag, const FInvSys_DragItemInstanceMessage& Message)
	{
		if (ItemInstance.IsValid() && Message.ItemInstance == ItemInstance)
		{
			NativeOnDragItem(Message.bIsDraggingItem);
		}
	};

	// 监听 Dragging 事件
	UGameplayMessageSubsystem& GameplayMessageSubsystem = UGameplayMessageSubsystem::Get(GetWorld());
	DragItemListenerHandle = GameplayMessageSubsystem.RegisterListener<FInvSys_DragItemInstanceMessage>(Inventory_Message_DragItem, MoveTemp(OnDraggingItemFunc));
}

void UInvSys_InventoryItemWidget::NativeDestruct()
{
	Super::NativeDestruct();

	GetWorld()->GetTimerManager().ClearTimer(ServerTimeoutHandle);
	DragItemListenerHandle.Unregister();
}

void UInvSys_InventoryItemWidget::NativeOnDragItem(bool bIsDraggingItem)
{
	// 如果物品已经被抓取了，就无法再次触发抓取事件
	if (bIsDraggingItem)
	{
		bIsEnableDragItem = false;
		OnDisableDragItemInstance();
	}
	else
	{
		bIsEnableDragItem = true;
		OnEnableDragItemInstance();
	}
}
