#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/SphereComponent.h"
#include "MyProject/Characters/BattleCharacter.h"
#include "Enemy_Zako.generated.h"

UCLASS()
class MYPROJECT_API AEnemy_Zako : public ACharacter
{
	GENERATED_BODY()

public:
	AEnemy_Zako();

protected:
	virtual void BeginPlay() override;

public:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Battle")
	USphereComponent* BattleTrigger;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	int32 EnemyID = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	FName EnemyUniqueName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	float MaxHP = 100.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Battle")
	float CurrentHP;

	UFUNCTION(BlueprintPure, Category = "Battle")
	float GetHealthPercentage() const;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION(BlueprintCallable, Category = "Battle")
	void TakeZakoDamage(float DamageAmount);
};