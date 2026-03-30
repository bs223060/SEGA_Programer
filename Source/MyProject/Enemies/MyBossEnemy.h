#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/BoxComponent.h" 
#include "MyProject/Characters/BattleCharacter.h"
#include "MyBossEnemy.generated.h"

UENUM(BlueprintType)
enum class EBossAttackType : uint8
{
    Normal      UMETA(DisplayName = "Normal Attack"),
    Heavy       UMETA(DisplayName = "Heavy Attack")
};

UCLASS()
class MYPROJECT_API AMyBossEnemy : public ACharacter
{
    GENERATED_BODY()

public:
    AMyBossEnemy();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Battle")
    UBoxComponent* BattleTrigger;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BossStats")
    int32 GiveExp = 60;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BossStats")
    float MaxHP = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BossStats")
    float CurrentHP;

    UPROPERTY(BlueprintReadOnly, Category = "BossStats")
    bool bIsEnraged = false;

    UPROPERTY(BlueprintReadWrite, Category = "BossStats")
    bool bIsCharging = false;

    UFUNCTION(BlueprintPure, Category = "BossStats")
    float GetHealthPercentage() const;

    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION(BlueprintCallable, Category = "BossLogic")
    EBossAttackType GetNextAttackType();

    UFUNCTION(BlueprintCallable, Category = "BossLogic")
    float CalculateAttackDamage();

    UFUNCTION(BlueprintCallable, Category = "BossLogic")
    void TakeBossDamage(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "BossLogic")
    void CheckHealthStatus();

    UFUNCTION(BlueprintCallable, Category = "BossLogic")
    void ExecuteSpecialAttack();
};