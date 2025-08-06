// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/InvSys_InventoryWidget.h"

#include "BaseInventorySystem.h"
#include "Components/InventoryObject/InvSys_BaseInventoryObject.h"

#include "Blueprint/WidgetTree.h"
#include "Components/InvSys_InventoryComponent.h"

void UInvSys_InventoryWidget::RefreshInventoryWidget(UInvSys_BaseInventoryObject* NewInventoryObject)
{
	check(NewInventoryObject)
	SetInventoryObject(NewInventoryObject);
	UE_CLOG(PRINT_INVENTORY_SYSTEM_LOG && InventoryObject == nullptr, LogInventorySystem, Error,
		TEXT("你必须在执行 Construct 函数前调用 SetInventoryObject 初始化属性."))
	OnRefreshWidget();
}

void UInvSys_InventoryWidget::NativeConstruct()
{
	UE_CLOG(PRINT_INVENTORY_SYSTEM_LOG && InventoryObject == nullptr, LogInventorySystem, Error,
		TEXT("你必须在执行 Construct 函数前调用 SetInventoryObject 初始化属性."))
	Super::NativeConstruct();
}

void UInvSys_InventoryWidget::SetInventoryComponent(UInvSys_InventoryComponent* NewInvComp)
{
	InventoryComponent = NewInvComp;
}

UInvSys_InventoryComponent* UInvSys_InventoryWidget::GetInventoryComponent() const
{
	return InventoryComponent.Get();
}

void UInvSys_InventoryWidget::SetInventoryObject(UInvSys_BaseInventoryObject* NewInventoryObject)
{
	InventoryObject = NewInventoryObject;
	if (InventoryObject)
	{
		SlotTag = InventoryObject->GetInventoryObjectTag();
		InventoryComponent = InventoryObject->GetInventoryComponent();
	}
}

