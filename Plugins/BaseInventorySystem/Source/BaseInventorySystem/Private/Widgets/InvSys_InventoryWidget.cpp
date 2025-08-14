// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/InvSys_InventoryWidget.h"

#include "BaseInventorySystem.h"
#include "Components/InventoryObject/InvSys_BaseInventoryObject.h"

#include "Blueprint/WidgetTree.h"
#include "Components/InvSys_InventoryComponent.h"

void UInvSys_InventoryWidget::RefreshInventoryWidget(UInvSys_BaseInventoryObject* NewInventoryObject)
{
	check(NewInventoryObject)
	if (NewInventoryObject)
	{
		InventoryObject = NewInventoryObject;
		SlotTag = InventoryObject->GetInventoryObjectTag();
		InventoryComponent = InventoryObject->GetInventoryComponent();
	}
	OnRefreshWidget();
}

void UInvSys_InventoryWidget::NativeConstruct()
{
	UE_CLOG(PRINT_INVENTORY_SYSTEM_LOG && InventoryObject == nullptr, LogInventorySystem, Error,
		TEXT("你必须在执行 Construct 函数前调用 RefreshInventoryWidget 初始化属性."))
	Super::NativeConstruct();
}

FGameplayTag UInvSys_InventoryWidget::GetSlotTag() const
{
	check(InventoryObject)
	return InventoryObject->GetInventoryObjectTag();
}

UInvSys_BaseInventoryObject* UInvSys_InventoryWidget::GetInventoryObject()
{
	check(InventoryObject);
	return InventoryObject;
}

