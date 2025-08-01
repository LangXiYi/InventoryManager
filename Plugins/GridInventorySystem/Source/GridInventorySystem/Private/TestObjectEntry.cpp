// Fill out your copyright notice in the Description page of Project Settings.


#include "TestObjectEntry.h"

#include "Net/UnrealNetwork.h"

void UTestObjectEntry::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	UObject::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UTestObjectEntry, Name, COND_None);
}

void UTestObjectEntry::OnRep_Name()
{
	UE_LOG(LogTemp, Error, TEXT("OnRep_Name ---> %s"), *Name);
}
